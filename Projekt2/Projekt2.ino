#include <rdm630.h>

#include "EEPROMAnything.h"
#include <SoftwareSerial.h>


// RFID
rdm630 rfid(2, 0);  //TX-pin of RDM630 connected to Arduino pin 6


// Config
class config_t {
public:
  unsigned long verifiedids[5] = {0, 0, 0, 0, 0};
} cfg;

void loadconfig()
{
  EEPROM_readAnything(0, cfg);
}

void saveconfig()
{
  EEPROM_writeAnything(0, cfg);
}

 // Screen liquid crystal
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

#define pin_motor_up 13
#define pin_motor_down 8

#define pin_button_right 12
#define pin_button_middle 11
#define pin_button_left 10

#define pin_wire_top 7
#define pin_wire_bottom 6

#define pin_distancesensor_trick 5
#define pin_distancesensor_signal 4

bool lock_menu = false;

unsigned long current_id;
void menu_0()
{
  if (lock_menu){ //Selected menu
    static int selected_id = 0;
  
    if (detect_lr_input(selected_id, 4)){
      lcd.clear();
    }
    
    lcd.setCursor (0,1);
    lcd.print("selid[" + String(selected_id+1) + "]:" + String(cfg.verifiedids[selected_id]));  
    
    lcd.setCursor (0,0);
    lcd.print("curid   :" + String(current_id));

    if (detect_m_input()){
      cfg.verifiedids[selected_id] = current_id;
      saveconfig();
      lcd.clear();
    }
  } else { //Just browsing menu's
    lcd.setCursor (0,0); 
    lcd.print("[menu]");  
    lcd.setCursor (0,1);
    lcd.print("Add id's");  
  }
}
void menu_1()
{
  if (lock_menu){ //Selected menu
    static int selected_id = 0;
  
    if (detect_lr_input(selected_id, 4)){
      lcd.clear();
    }
    
    lcd.setCursor (0,1);
    lcd.print("selid[" + String(selected_id+1) + "]:" + String(cfg.verifiedids[selected_id]));  
    
    lcd.setCursor (0,0);
    lcd.print("curid   :" + String(current_id));

    if (detect_m_input()){
      cfg.verifiedids[selected_id] = 0;
      saveconfig();
      lcd.clear();
    }
  } else { //Just browsing menu's
    lcd.setCursor (0,0);
    lcd.print("[menu]");  
    lcd.setCursor (0,1);
    lcd.print("Delete id's");  
  }
}

bool detect_menu()
{
  static bool pin_button_rightandleft_tgl = false;
  bool value = false;
  
  //Right 
  if (digitalRead(pin_button_right) == HIGH and digitalRead(pin_button_left) == HIGH and not pin_button_rightandleft_tgl){
    pin_button_rightandleft_tgl = true;
    // At press
    value = true;
    
  }
  if (digitalRead(pin_button_right) == LOW and digitalRead(pin_button_left) == LOW and pin_button_rightandleft_tgl){
    pin_button_rightandleft_tgl = false;
    // After press
  }

  return value;
}

bool detect_m_input()
{
  static bool pin_button_middle_tgl = false;
  bool value = false;
  
  //Right 
  if (digitalRead(pin_button_middle) == HIGH and not pin_button_middle_tgl){
    pin_button_middle_tgl = true;
    // At press
    value = true;
    
  }
  if (digitalRead(pin_button_middle) == LOW and pin_button_middle_tgl){
    pin_button_middle_tgl = false;
    // After press
  }

  return value;
}

bool detect_lr_input(int &curmen, int maxmen)
{
  static bool pin_button_right_tgl = false;
  static bool pin_button_left_tgl = false;
  bool changed = false;
  
  //Right 
  if (digitalRead(pin_button_right) == HIGH and not pin_button_right_tgl){
    pin_button_right_tgl = true;
    // At press
    changed = true;
    //menu_right
    curmen++;
    if (curmen > maxmen){
      curmen = 0;
    }
  }
  if (digitalRead(pin_button_right) == LOW and pin_button_right_tgl){
    pin_button_right_tgl = false;
    // After press
  }
  // Left
  if (digitalRead(pin_button_left) == HIGH and not pin_button_left_tgl){
    pin_button_left_tgl = true;
    changed = true;
    //menu_left
    curmen--;
    if (curmen < 0){
      curmen = maxmen;
    }
  }
  if (digitalRead(pin_button_left) == LOW and pin_button_left_tgl){
    pin_button_left_tgl = false;
  }

  return changed;
}

void menu()
{
  static int current_menu = 0;

  if (not lock_menu){
    if (detect_lr_input(current_menu, 1)){
      lcd.clear();
    }
    if (detect_m_input()){
      lock_menu = true;
      lcd.clear();
    } 
  } else {
    if (detect_menu()){
      lock_menu = false;
    }
  }
  
  if (current_menu == 0) { //add id
    menu_0();
  }
  if (current_menu == 1) { //delete id
    menu_1();
  }
}

/*void blink(int n = 1) 
{
  for(int i = 0; i < n; i++) {
    digitalWrite(pin_led, HIGH);
    delay(200);
    digitalWrite(pin_led, LOW);
    delay(200);
  }
}*/

unsigned long time_close;
void check_gate()
{
  static bool shouldopen = false;
  
  if (millis() >= time_close) {//Should close
    shouldopen = false;
  } else {//Should open
    shouldopen = true;
  }

  if (shouldopen){
    //Open
    digitalWrite(pin_motor_down, not LOW); //H-Bridge, reverse inputs
    
    if (digitalRead(pin_wire_top) == HIGH){ //still connected
      Serial.println("Opening");
      digitalWrite(pin_motor_up, not HIGH);
    } else {// not connected
      digitalWrite(pin_motor_up, not LOW);
    }
  } else {
    //Close
    digitalWrite(pin_motor_up, not LOW);
    
    if (digitalRead(pin_wire_bottom) == HIGH){ 
      Serial.println("Closing");
      digitalWrite(pin_motor_down, not HIGH);
    } else {
      digitalWrite(pin_motor_down, not LOW);
    }
  }
}

void check_cat()
{
    byte data[6];
    byte length;

    if(rfid.available()){
        rfid.getData(data,length);
        /*Serial.println("Data valid");
        for(int i=0;i<length;i++){
            Serial.print(data[i],HEX);
            Serial.print(" ");
        }
        Serial.println();*/
        //concatenate the bytes in the data array to one long which can be 
        //rendered as a decimal number
        current_id = 
          ((unsigned long int)data[1]<<24) + 
          ((unsigned long int)data[2]<<16) + 
          ((unsigned long int)data[3]<<8) + 
          data[4];              
        Serial.print("decimal CardID: ");
        Serial.println(current_id);
        for (int x=0; x< sizeof(cfg.verifiedids)/sizeof(cfg.verifiedids[0]); x++){
            if (cfg.verifiedids[x] != 0 and cfg.verifiedids[x] == current_id){
                //open for 10 secs from this point when we have seen the cat
                time_close = millis() + 1000 * 20;
                Serial.println("Open");
                break;
            }
        }
    }

  //Sonic Distance Sensor
  // Clears the trigPin
  digitalWrite(pin_distancesensor_trick, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(pin_distancesensor_trick, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_distancesensor_trick, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(pin_distancesensor_signal, HIGH);
  
  // Calculating the distance
  long distance= duration/29/2;
  
  //Checks for cat on the inside
  if (distance <= 30){
    time_close = millis() + 1000 * 20;
    
    // Prints the distance on the Serial Monitor
    //Serial.print("Distance: ");
    //Serial.println(distance);
  }

}

void setup() {
  Serial.begin(9600);

  pinMode(pin_motor_up, OUTPUT);
  pinMode(pin_motor_down, OUTPUT);
  digitalWrite(pin_motor_up, not LOW);
  digitalWrite(pin_motor_down, not LOW);
  
  pinMode(pin_button_right, INPUT);
  pinMode(pin_button_middle, INPUT);
  pinMode(pin_button_left, INPUT);
  
  pinMode(pin_wire_top, INPUT);
  pinMode(pin_wire_bottom, INPUT);

  pinMode(pin_distancesensor_trick, OUTPUT); // Sets the trigPin as an Output
  pinMode(pin_distancesensor_signal, INPUT); // Sets the echoPin as an Input
  
  loadconfig();
  //Serial.println(cfg.starttime);
  
  //cfg.starttime = 1337;
  //saveconfig();
  //cfg.verifiedids[0]=2486204;

  // Screen
  lcd.begin (16,2);

  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();
  
  lcd.setCursor (0,0);//Pos, line
  
  //lcd.print("Welcome to catdoor!");
  //delay(500);
  //lcd.clear();

  //RFID
  rfid.begin();

  Serial.println("Begin");
}

void loop() {
  // put your main code here, to run repeatedly:
  menu();
  check_cat();
  check_gate();
}
