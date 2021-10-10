#define INIT_KEY 100
#include <EEPROM.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <HX711_ADC.h>
#include <Keypad.h>
#include<GyverOS.h>

 
const int HX711_dout = 9; // pin for Cell dout 
const int HX711_sck = 2; //pin for Cell clock

uint32_t  stabilizingtime = 1000; // wait stablizing time 


LiquidCrystal_I2C lcd(0x27,20,4);  // lcd initialization 
HX711_ADC Cell(HX711_dout, HX711_sck) ; // weight cell initialization 

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char Keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},       // Key pad iniatialization 
  {'*', '0', '#', 'D'}
};


byte rowPins[ROWS] = {3, 4, 5, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 8, 10,11 }; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(Keys), rowPins, colPins, ROWS, COLS);

GyverOS<3> OS;

  String string = "";  // string for storing values from keypad 
  char key ; // for keypad 
  uint32_t tmr,tmr_wait,tmr_disp,tmr_lcd,tmr_remove,tmr_sleep ; // for keypad hold 
  bool flag1,flag2,flag3,flag4,flag5,flag6  = 0 ; // pole for normal scale
  byte Cursor = 0 ; 
  float Sum,Cell_measured = 0  ; // weight measured 
  const int calibr_address = 3 ;
 const float calibration_value = 20.00 ; 
 
void setup() {
  lcd.init();   
  
  lcd.init(); 
  lcd.backlight();
  Serial.begin(9600);

  scale_initial(); // initialize scale 


  OS.attach(0, First_scaleDisplay, 0);   // first task 
  OS.attach(1, Unit_price, 0);  // second task 
  OS.attach(2,Total_sum,0); 

OS.stop(1);
OS.stop(2);
}

void loop() {
 
 OS.tick();  // start task manager 

}


///////Scale Initialization  ////////
  void scale_initial()
  {
    
    
  if(EEPROM.read(2) != INIT_KEY )
{
EEPROM.write(2,INIT_KEY);




}
EEPROM.put(calibr_address,calibration_value);
  
     lcd.backlight();
     
Cell.begin() ; // begin weight cell 

 lcd.print("STARTING........! "); // print for 1 second 

  lcd.clear() ; // clear lcd 
  Cell.start(stabilizingtime,true); // begin cell of weight 
  
  if(Cell.getTareTimeoutFlag() == true) // if wiring problem of cell than error 
{
	lcd.setCursor(0,0); 
	lcd.print("wiring problem"); 
  Serial.println( "Wiriing problem  " );  
}

Cell.tare();  // empty the scale 

 if(Cell.getTareStatus() == true)
  {
   EEPROM.get(calibr_address,calibration_value);
 Cell.setCalFactor(calibration_value);  // calibration factor for setting scale first weight 
  
  }
  
  }
/////////////////****************//////////////////

void First_scaleDisplay()
{
  
  static bool  flag = 0 ;
  float value=0;
   
  lcd.clear() ; 
   
  while(1)
  {
 lcd.backlight();
 
  if (Cell.update())  flag = true ;
  
  if (flag  )
  {
    value  = Cell.getData(); // cell measurment value 
    value = value * 0.001 ; // i multiplied grams value to 0.001 to get kg value 
    value = constrain(value,0,200);  // max values 
   float gram = value * 1000 ;  // to get grams values back 
   
 lcd.setCursor(0,0);
 lcd.print("KILOGRAM :"); 
 lcd.setCursor(0,1); 
   lcd.print(value); // we get kg filtered  values here 
  lcd.setCursor(0,2); 
  lcd.print("GRAMS : ");   
    lcd.setCursor(0,3);
    lcd.print(gram); // we get grams values with filtered 
    
   
    key = customKeypad.getKey() ;
   if(key == '*')
   {
    flag3 = 1 ;
    tmr_wait = millis() ;
    lcd.clear() ; 
    
   }
   if( flag3 && customKeypad.getState() == HOLD)
{
  if(millis() - tmr_wait >= 3000)
  {
    lcd.clear() ;
    Cell.tare();
    flag3 = 0 ;
  }
}
}
  
  if(key == 'A')
 {
  lcd.clear() ;
 flag1 = 1 ; 
 tmr = millis() ;
 Serial.print("flag1 ");Serial.println(flag1);
 
 }
 
 if( flag1 && customKeypad.getState() == HOLD)
{
  if(millis() - tmr >= 3000)
  {
  lcd.clear() ;
   flag1 = 0; 
  lcd.setCursor(0,0) ; 
  lcd.print("SCALE SELECT  ");
  delay(2000);
  lcd.clear() ;
 Serial.print("flag1 ");Serial.println(flag1);
  break ;
}

}
  
  }
while(1 )
{
  
 key = customKeypad.getKey() ; 
 
    if(key == 'A')
    {
      
       lcd.clear();
        Cursor += 1 ;
        
       if(Cursor >1)Cursor = 0 ; 
       
 Serial.print("Cursor");Serial.println(Cursor);
 lcd.setCursor(0,Cursor);
 
    }
      lcd.setCursor(0,Cursor);
     lcd.print("->");  
  lcd.setCursor(2,0); 
  lcd.print("NORMAL ") ;
  
  lcd.setCursor(2,1 ); 
  lcd.print("COMMERCIAL "); 
  
     if(Cursor == 0 && key == '#' )
       {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("NORMAL SCALE") ; 
        delay(1000); 
         OS.stop(1);
         OS.start(0);
         OS.stop(2);
        Serial.print("flag1 ");Serial.println(flag1);
        
         break ;
       
       }
       else if(Cursor == 1 && key == '#'  )
       {
          lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("COMMERCIAL SCALE   ") ; 
        delay(1000);
        Serial.print("flag1 ");Serial.println(flag1);
       
      OS.stop(0);
         OS.start(1);
          OS.stop(2);
         break ;
       }
}
  

  
}


void Unit_price() 
{ 
  lcd.backlight();
 lcd.clear() ;

static float total_price = 0 ; 
 static float kg = 0.0 ; 
static float Cell_measured = 0.0; 

 static int convert ;
String str1 = "" ;  

 while(1)
 { 
  
  key = customKeypad.getKey();  
  lcd.setCursor(0,2);
  lcd.print("INPUT PRICE:"); 
   
  if(key )
  {
    
 lcd.setCursor(13,2);
 string += key ;
 lcd.print(string );  
 
  if(key == '*' )
 {
  lcd.clear() ; 
  string =  "" ;
 }

  }
 else
 {
   
 convert = string.toInt() ; 
 
  Cell.update();
  Cell_measured = Cell.getData();
  Cell_measured =constrain(Cell_measured,0,200000);
  kg = Cell_measured * 0.001;
  total_price  = Cell_measured * convert / 1000;
  
lcd.setCursor(0,0); 
lcd.print("GRAMS"); 
lcd.setCursor(0,1); 
lcd.print(Cell_measured);
lcd.setCursor(10,0); 
lcd.print("KG");
lcd.setCursor(10,1);
lcd.print(kg) ;  
lcd.setCursor(0,3);
lcd.print("TOTAL PRICE:"); 
lcd.setCursor(13,3 ); 
lcd.print(total_price); // Total price measured


}


if(key == 'B')
{
  Sum += total_price ; 
  EEPROM.put(8,Sum);
 Serial.println(total_price );
 Serial.println(Sum);
 
}


if(key == 'D')
{
  flag6  = 1 ;
  tmr_remove = millis()  ;
  
}
if( flag6 && customKeypad.getState() == HOLD)
{
if( millis()- tmr_remove >=3000)
{
  Sum = 0 ;
  EEPROM.put(8,Sum);
   lcd.clear() ;
   
  flag6 = 0 ;
 lcd.home() ;  
 lcd.print("Record Cleared....! ") ; 
 delay(1000); 
 lcd.clear() ; 
 break ; 
  
}

}

if(key == 'C')
 {
  
  flag4 = 1 ; 
  tmr_disp = millis() ;
 
 }
 
 if( flag4 && customKeypad.getState() == HOLD)
 {
  if(millis() - tmr_disp >= 3000)
  {
     flag5 = 1 ;
    lcd.clear() ; 
    flag4 = 0 ;
    tmr_lcd = millis() ;
    EEPROM.get(8,Sum);
    Total_sum(Sum);
    string =  "" ;
    OS.stop(0);
    OS.stop(1);
    OS.start(2);
    break; 
  }
  
 }
 

  if(key == 'A')
 {
 
 flag2 = 1 ;
 tmr = millis() ;
 
  
 }
 
 if( flag2 && customKeypad.getState() == HOLD)
{
  if(millis() - tmr >= 3000)
  {
    lcd.clear() ;
  flag2 = 0;
  lcd.setCursor(0,0) ; 
  lcd.print("SCALE SELECT  ");
  delay(2000);
  lcd.clear() ;
  break ;
}
}
  
}

   
while(1 )
{
  
 key = customKeypad.getKey() ; 
 
    if(key == 'A')
    {
      
       lcd.clear();
        Cursor += 1 ;
       if(Cursor >1)Cursor = 0 ; 
 lcd.setCursor(0,Cursor);
 
    }
      lcd.setCursor(0,Cursor);
     lcd.print("->");  
  lcd.setCursor(2,0); 
  lcd.print("COMMERCIAL ") ;
  
  lcd.setCursor(2,1 ); 
  lcd.print("NORMAL "); 
  
     if(Cursor == 0 && key == '#' )
       {
        lcd.clear();
         lcd.setCursor(0,0);
        lcd.print("Commercial ") ; 
        delay(1000); 
        
         OS.stop(0);
         OS.start(1);
         break ;
       
       }
       else if(Cursor == 1 && key == '#'    )
       {
          lcd.clear();
         lcd.setCursor(0,0);
        lcd.print("NORMAL ") ; 
        delay(1000); 
         OS.stop(1);
         OS.start(0);
         break ;
       }
}



}

void Total_sum(float val)
{
  
while(millis() - tmr_lcd < 5000 )
{
  
  lcd.home()  ;
  lcd.print("Total Sell") ; 
  
  lcd.setCursor(0,1) ; 
  if( val > 0 && flag5 )
  {
    EEPROM.get(8,Sum);
    lcd.print(val)  ;
   
     flag5 = 0 ; 
     
    if(millis()  - tmr_lcd >4000) lcd.clear() ;
   
  
  }
   Serial.println(val);
}

lcd.clear() ;

OS.stop(0); 
OS.start(1); 
OS.stop(2);

 
}
