/* I2C LCD with Arduino example code. More info: https://www.makerguides.com */

#include <Wire.h> // Library for I2C communication

// LCD Display 
#include <LiquidCrystal_I2C.h> // Library for LCD

// !!! SET HERE !!!  The elevation above main sealevel in meter where you cook your egg !!!!
#define ELEVATION 285

// Rotary switch with push button to navigate the menue and select.
#define rotary_a 2
#define rotary_b 3
#define rotary_button 4

// I used a PC Mainboard laudspeeker to generate click sound and final finish sounds
#define beep A0


enum {
  MENU_WELCOME = 0,
  MENU_EI_GEWICHT,
  MENU_EI_TEMP,
  
  MENU_EI_KOCH_INFO,
  MENU_EI_KOCHEN
 
};

int menu_state = MENU_WELCOME;

float ei_wight_count = 55; 
float ei_temp_count = 25.0;
float ei_end_temp = 64.5;  // 63.5 was a bit to undercooked
int ei_koch_time = 0; 

// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4); // Change to (0x27,16,2) for 16x2 LCD.

void setup() {
  // Initiate the LCD:
  lcd.init();
  lcd.backlight();

  pinMode (rotary_a,INPUT);
  pinMode (rotary_b,INPUT);
  pinMode (rotary_button,INPUT);

  pinMode (beep,OUTPUT);

  // Debug output
  Serial.begin (9600);
}

void loop() {
  do_lcd();
}

void do_lcd(void) {
  static unsigned long last_sec;
  float tmp;
  
  switch(menu_state) {
    case MENU_WELCOME:
                        lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
                        lcd.print("The perfect     "); // Print the string "Hello World!"
                        lcd.setCursor(0, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
                        lcd.print("Egg Timer       ");
                        delay(2500);
                        menu_state = MENU_EI_GEWICHT;
                        
    
                        break;
    case MENU_EI_GEWICHT: // Set weight of the egg
                        lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
                        lcd.print("Step 1   Ei");
                        lcd.setCursor(0, 1); // Set the cursor on the first column and first row.
                        lcd.print("Gewicht: "); 
                        lcd.print((int)ei_wight_count);
                        lcd.print("g");
                        while (do_Rotary_button() == 0) { 
                          tmp = do_Rotary(ei_wight_count, 0);
                          if(tmp != 0) {
                            ei_wight_count = tmp;
                            lcd.setCursor(9, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
                            lcd.print((int)ei_wight_count);
                            lcd.print("g");
                            
                          }
                          //Serial.println(do_Rotary_button());
                        }
                        //Serial.println("EXIT:... to next step");
                        menu_state = MENU_EI_TEMP;
                        delay(100);
                        break;

    case MENU_EI_TEMP:  // Set aktual temperature of the egg
                        lcd.setCursor(0, 0);
                        lcd.print("Step 2   Ei");
                        lcd.setCursor(0, 1);
                        lcd.print("Temperatur: ");
                        lcd.print(ei_temp_count);
                        //lcd.print("°");
                        while (do_Rotary_button() == 0) { 
                          tmp = do_Rotary(ei_temp_count, 1);
                          if(tmp != 0) {
                            ei_temp_count = tmp;
                            lcd.setCursor(12, 1);
                            lcd.print(ei_temp_count);
                            
                          }
                        }
                        menu_state = MENU_EI_KOCH_INFO;
                        ei_koch_time = calc_egg_time();
                        delay(100);
                        break;

    case MENU_EI_KOCH_INFO:
                        lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
                        lcd.print("Kochzeit: "); // Print the string "Hello World!"
                        lcd.print(ei_koch_time);
                        lcd.print("s");
                        lcd.setCursor(0, 1); // Set the cursor on the first column and first row.
                        lcd.print("Button -> Start "); // Print the string "Hello World!"
                        while (do_Rotary_button() == 0) { 

                        }
                        menu_state = MENU_EI_KOCHEN;
                        delay(100);
                        break;

    case MENU_EI_KOCHEN:
                        lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
                        lcd.print("Kochzeit: "); // Print the string "Hello World!"
                        lcd.print(ei_koch_time);
                        lcd.print("s");
                        lcd.setCursor(0, 1); // Set the cursor on the first column and first row.
                        lcd.print("Button -> Exit  "); // Print the string "Hello World!"
                        //lcd.print("°");
                        while (do_Rotary_button() == 0) {
                          if(millis() >= (last_sec + 1000)) {
                            last_sec = millis();
                            ei_koch_time--;
                            lcd.setCursor(10, 0); // Set the cursor on the first column and first row.
                              if(ei_koch_time < 100 && ei_koch_time >= 10){
                                lcd.print(" ");
                              } else if(ei_koch_time < 10) {
                                lcd.print("  ");
                              } else {
                                // do nothing
                              }
                            
                            lcd.print(ei_koch_time);

                            if((ei_koch_time <= 5) && (ei_koch_time > 0))
                             //do_beep();
                             tone(beep, 500, 100);
                             else
                             tone(beep, 500, 1);
                             
                              
                            
                          }
                          if(ei_koch_time <= 0) {
                           tone(beep,500,1500);
                           break;
                          }
                        }
                        menu_state = MENU_WELCOME;
                        delay(100);
                        break;

    
    }
}


int calc_egg_time(void) {
  int ret;
  
  float t_wasser = (100 - ((float)142 / (float)ELEVATION));
    Serial.println(t_wasser);
   ret = (int)(465 * pow(ei_wight_count, 2/3) * log((float)0.76 * ((t_wasser - ei_temp_count) / (t_wasser - ei_end_temp))));
    Serial.println(ret);
    return ret;
}

int do_Rotary_button(void) {
  int akt;
  static int last;
  int ret;
  
    ret = 0;
    akt = !digitalRead(rotary_button);
    if(akt != last) {  // cahnge
      last = akt;
      if(akt == 1)     // press
        ret = 1;
    }
    return ret;
}

 float do_Rotary(float counter, int dot) {
 static int aState;
 float ret;
 static int aLastState; 
 int direct; 

     ret = 0;
     aState = !digitalRead(rotary_a); // Reads the "current" state of the outputA
   // If the previous and the current state of the outputA are different, that means a Pulse has occured
   if (aState != aLastState) {
    if( aState == 1) {    
     // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
     direct = !digitalRead(rotary_b);
     if (direct != 1) {
      
      if(dot == 1)
        counter += 0.1;
      else 
       counter ++;
     } else {
      if(dot == 1)
        counter -= 0.1;
      else 
       counter --;
     }
     Serial.println(counter);
       ret = counter;
    }
     aLastState = aState; // Updates the previous state of the outputA with the current state
   }
   return ret;
}
