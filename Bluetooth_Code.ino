#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

//Allows us to use other pins as RX and TX pins
SoftwareSerial Bluetooth(2, 3); //TX, RX

//Pins for LEDs and Button
#define Button 8

int buttonState = 0; //State of the button

// Score variables
int redScore = 0, blueScore = 0;
int totalRedScore = 0, totalBlueScore = 0;

// Round state variables
enum RoundState { ACTIVE, MANUAL, CONFIRM };
RoundState roundState = ACTIVE;

char sending = ' '; //Data sent via bluetooth
char receiving = ' '; //Data received via bluetooth

// Button States
bool R_state = false;
bool B_state = false;
bool zero_state = false;


// LCD Initialization
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);                     

void setup() {

  lcd.begin(16, 2);
  //PinModes for TX, RX, and Button
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  pinMode(Button, INPUT);

  //Starts both Serial and Bluetooth communication
  Serial.begin(38400);
  Serial.println("\n Serial Print is Ready");
  Bluetooth.begin(38400);
  Serial.println("\n Bluetooth Started");

  updateLCD();
}

/*
Send/Recieve Scoring Library
--------------------------------------
 0 - Nothing is changed on scoreboard
 1 - Red Team Score + 3
 2 - Blue Team Score + 3

*/


void loop() {
  // put your main code here, to run repeatedly:

  // We listen to the bluetooth with this
  if (Bluetooth.available()){
    receiving = Bluetooth.read();
    Serial.write(receiving);
  }

  //What we use to read the button
  buttonState = digitalRead(Button);
  if (buttonState == LOW) {
    sending = '1';
    R_state = true;
  } else {
    sending = '0';
    zero_state = true;
  }  

  //What we do with the Bluetooth data
  if (receiving == '1') {
    // Increase Score
    receiving = '0';
    B_state = true;
  }
  else if (receiving == '0') {
    // Do Nothing
    receiving = '0';
    zero_state = true;
  }

  while (zero_state == true){
    if(R_state == true){
        redScore = redScore + 3;
      }
      if(B_state == true){
        blueScore = blueScore + 3;
      }

      R_state  = false;
      B_state  = false;
      zero_state  = false;
  }

  // We write to the bluetooth with this this
  if (Serial.available()){;}
    Serial.print(sending); 
    Bluetooth.print(sending);

  //Cleans us serial monitor
  Serial.print("\n");
  delay(10);

  updateLCD();
}

// For LCD Screen
void updateLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RED:");
    lcd.setCursor(4, 0);
    lcd.print(totalRedScore);
    lcd.setCursor(9, 0);
    lcd.print("BLUE:");
    lcd.setCursor(14, 0);
    lcd.print(totalBlueScore);
    lcd.setCursor(2, 1);
    lcd.print((roundState == ACTIVE || roundState == MANUAL) ? redScore : 0);
    lcd.setCursor(10, 1);
    lcd.print((roundState == ACTIVE || roundState == MANUAL) ? blueScore : 0);
}

