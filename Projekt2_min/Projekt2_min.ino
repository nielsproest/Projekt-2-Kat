
//definere om man kan der er et rfid i nærheden
extern bool rfid_available();
//får rfid'et
extern unsigned long rfid_get();
//får afstanden som sonic sensoren ser
extern int sonic_get_distance();


class config_t {
public:
  unsigned long verifiedids[5] = {0, 0, 0, 0, 0};
} cfg;

//en funktion som læser konfigurationen, den inkludere katte id'er
void loadconfig()
{
  EEPROM_readAnything(0, cfg);
}

//en funktion til at gemme konfigurationen
void saveconfig()
{
  EEPROM_writeAnything(0, cfg);
}

void setup() {
  //starter en seriel forbindelse som bruges til at få data til computeren som bruges til debugging
  Serial.begin(9600);

  //her indstillies vores pins til enten OUTPUT (skrive) eller INPUT (læse)
  pinMode(pin_motor_up, OUTPUT);
  pinMode(pin_motor_down, OUTPUT);
  //H-Broen bruges omvendt, so vi skriver bare not foran for at det bliver læt at læse
  digitalWrite(pin_motor_up, not LOW);
  digitalWrite(pin_motor_down, not LOW);
  
  pinMode(pin_button_right, INPUT);
  pinMode(pin_button_middle, INPUT);
  pinMode(pin_button_left, INPUT);
  
  pinMode(pin_wire_top, INPUT);
  pinMode(pin_wire_bottom, INPUT);

  pinMode(pin_distancesensor_trick, OUTPUT); // Sets the trigPin as an Output
  pinMode(pin_distancesensor_signal, INPUT); // Sets the echoPin as an Input

  // Læs konfigurationen
  loadconfig();

  // Start skærmen
  lcd.begin (16,2);

  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();
  
  lcd.setCursor (0,0);//Pos, line

  //Start rfid sensoren
  rfid.begin();

  Serial.println("Begin");
}

unsigned long time_close;
void check_gate()
{
  static bool shouldopen = false;
  
  if (millis() >= time_close) {//checker om tiden er stårere end tiden til at lukke, hvis den er skal den lukke
    shouldopen = false;
  } else {//Ellers skal den åbnes
    shouldopen = true;
  }

  if (shouldopen){
    //Open
    //hvis den skal åbne skal h-broen ikke fører motoren nedad
    digitalWrite(pin_motor_down, not LOW); //H-Bridge

    //Ser om der en elektrisk forbindelse, hvis der ikke er skal den fortsætte opad
    if (digitalRead(pin_wire_top) != HIGH){
      Serial.println("Opening");
      digitalWrite(pin_motor_up, not HIGH);
    } else {//ellers skal den ikke trække
      digitalWrite(pin_motor_up, not LOW);
    }
  } else {
    //Close
    //gør det samme
    digitalWrite(pin_motor_up, not LOW);
    
    if (digitalRead(pin_wire_bottom) != HIGH){ 
      Serial.println("Closing");
      digitalWrite(pin_motor_down, not HIGH);
    } else {
      digitalWrite(pin_motor_down, not LOW);
    }
  }
}


void check_cat()
{
    if(rfid_available){
        current_id = rfid_get();     
        //så søger vi vores liste over rfider
        for rfid in cfg.verifiedids do{
            if (rfid != 0 and rfid == current_id){
                //nu er der en kat så deen skal åbnes indtil om 20 sekunder
                time_close = millis() + 1000 * 20;
                break;
            }
        }
    }

  //Får den nuværende distance fra sonic sensoren
  if (sonic_get_distance() <= 30){
    //sætter den til at åbne i 20 sekunder
    time_close = millis() + 1000 * 20;
  }

}

unsigned long current_id;

//viser menu 0
extern void menu_0();
//viser menu 1
extern void menu_1();

//checker om vi både trykker på højre og venstre
extern bool detect_menu();

//checker om vi trykker på midter knappen
extern bool detect_m_input();

//her ser vi om højre og venstre knap bliver trykket på, hvis højre så tilføj 1, hvis venstre så træk 1 fra
extern bool detect_lr_input(int &curmen, int maxmen);

void menu()
{
  //den nuværende menu som bliver gemt
  static int current_menu = 0;

  if (not lock_menu){ //hvis vi burde browse for menuer
    //sæt nuværende id til 0 (forhindre at idet forbliver i menuen)
    current_id = 0;
    //checker om vi skifter menu, (sætter current_menu op og ned)
    if (detect_lr_input(current_menu, 1)){
      //gør klar hvis vi skal vise en ny menu
      lcd.clear();
    }
    //hvis vi klikker på midterknappen skal vi ind i menuen
    if (detect_m_input()){
      //vi burde ikke browse
      lock_menu = true;
      lcd.clear();
    } 
  } else {
    //hvis vi er inde i en menu og trykker på både højre og venstre så skal vi browse igen
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

void loop() {
  menu();
  check_cat();
  check_gate();
}
