/*
  File: Full_Game_v8.py
  Authors: Michael Appael, Lukasz Janik, Eamonn O'Connell
  Date: April 7, 2025
  Description: The automatic cornhole scoring system tracks the cornhole bags to automatically score and display the game results in a standard game of cornhole.
  Version: 8.0
*/

// Libraries Included
#include <LiquidCrystal.h>
#include <Pixy2.h>
#include <HardwareSerial.h>

// Create an alias for Serial3 and called it Bluetooth
HardwareSerial& Bluetooth = Serial3;

// Pinout for Pixy Camera
#define redSig 3
#define blueSig 2

Pixy2 pixy;

// Define button pins
const int redIncreasePin = 7;
const int redDecreasePin = 8;
const int blueIncreasePin = 9;
const int blueDecreasePin = 10;
const int endRoundPin = 6;
const int resetButtonPin = 38; // Reset/New Game Button

// LED pins for mode indication
const int activeScoringLEDPin = 40; // LED for ACTIVE scoring mode
const int manualScoringLEDPin = 41; // LED for MANUAL scoring mode

// Score variables
int redScore = 0, blueScore = 0; // Round Score
int totalRedScore = 0, totalBlueScore = 0; // Game Score

// Store previous OnRed & OnBlue for comparison
int RedReg = 0; 
int BlueReg = 0; 

// OnBoard Scoring for Red & Blue teams
int OnRed = 0;
int OnBlue = 0;

int count = 0; //Counter for the camera

// Round state variables
enum RoundState { ACTIVE, MANUAL, CONFIRM };

// Set starting state of board, one should be CONFIRM & other should be ACTIVE
RoundState StartingState = CONFIRM; // CHANGE THIS DEPENDING ON BOARD
RoundState roundState = StartingState;

// Strings to send/recieve Bluetooth 
String sending = ""; //Data sent via Bluetooth
String receiving = ""; //Data received via Bluetooth

// State to see if the board should read Bluetooth
bool ReadingState = false; // FALSE to not read | TRUE to read

// LCD Initialization
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Color Sensor's Pinout
#define s0 22
#define s1 23
#define s2 24
#define s3 25
#define out 26

// LEDs for Color Sensor
#define R_LED 32
#define G_LED 34
#define B_LED 36

// RGB Values for Color Sensor
int Red = 0, Blue = 0, Green = 0;

/*
  Function: setup()

  Description: Initializes the LCD, sets up pin modes for various components, 
               starts serial communication, initializes the Pixy camera, and 
               updates the LCD and LEDs.

  Parameters: None

  Returns: None
*/
void setup() {
  lcd.begin(16, 2);
  pinMode(14, INPUT);
  pinMode(15, OUTPUT);
  pinMode(redIncreasePin, INPUT_PULLUP);
  pinMode(redDecreasePin, INPUT_PULLUP);
  pinMode(blueIncreasePin, INPUT_PULLUP);
  pinMode(blueDecreasePin, INPUT_PULLUP);
  pinMode(endRoundPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);

  // Sensor pins
  pinMode(s0, OUTPUT);  
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);

  //LED setup
  pinMode(activeScoringLEDPin, OUTPUT);
  pinMode(manualScoringLEDPin, OUTPUT);

  // LED Pins
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);

  Serial.begin(38400);
  Bluetooth.begin(38400);
  digitalWrite(s0, HIGH);
  digitalWrite(s1, HIGH);

  pixy.init();

  updateLCD();
  updateLEDs();
}

/*
  Function: loop()

  Description: Main loop function that handles Bluetooth input, end round button, reset button, 
               and updates scores and states based on the current round state. 
               Calls functions to process input, update LCD and LEDs, and manage scoring.

  Parameters: None

  Returns: None
*/
void loop() {
  if (Bluetooth.available()){
    receiving = Bluetooth.readStringUntil('.');
    Serial.println(receiving);
    processInput(receiving);
  }
 
  static bool lastEndRoundState = HIGH;
  bool endRoundState = digitalRead(endRoundPin);

  // Handle end round button
  if (endRoundState == LOW && lastEndRoundState == HIGH) {
    if (roundState == ACTIVE) {
      roundState = MANUAL;
    } else if (roundState == MANUAL) {
      int scoreDifference = abs(redScore - blueScore);
      if (redScore > blueScore) {
        totalRedScore += scoreDifference;
      } else if (blueScore > redScore) {
        totalBlueScore += scoreDifference;
      } 
      sending = "Switch.";
      redScore = 0;
      blueScore = 0;
      roundState = CONFIRM;
    } else if (roundState == CONFIRM) {
      //roundState = ACTIVE;
    }
    updateLCD();
    updateLEDs();
  }
  lastEndRoundState = endRoundState;

  // Check reset button
  static unsigned long buttonPressTime = 0;
  if (digitalRead(resetButtonPin) == LOW) {
    if (buttonPressTime == 0) {
      buttonPressTime = millis(); // Start timing the button press
    } else if (millis() - buttonPressTime >= 3000) {
      // Reset scores if button is held for 3 seconds
      redScore = 0;
      blueScore = 0;
      totalRedScore = 0;
      totalBlueScore = 0;
      RedReg = 0; 
      BlueReg = 0; 
      OnRed = 0;
      OnBlue = 0;
      sending = "Reset.";
      roundState = StartingState;
      updateLCD();
      updateLEDs();
      buttonPressTime = 0; // Reset the timer
    }
  } else {
    buttonPressTime = 0; // Reset if the button is not held
  }

  if (roundState == ACTIVE) {
    GetColors();
    ReadingState = false;
    if (count == 100){
      OnBoard();
      count = 0;
    }
    count += 1;
  } else if (roundState == MANUAL) {
    ReadingState = false;
    handleScoring();
  } else if (roundState == CONFIRM){
    ReadingState = true;
  }

  if (sending == ""){
    // Make it send entire score packet
    sending = String(redScore) + "," + String(blueScore) + ".";
  }

  if (Serial.available()){;}
  Serial.print(sending);
  Bluetooth.print(sending);
  sending = "";
}

/*
  Function: handleScoring()

  Description: Handles the scoring logic for red and blue teams based on button states. 
               Updates the scores and LCD display accordingly.

  Parameters: None

  Returns: None
*/
void handleScoring() {
  static bool lastRedIncreaseState = HIGH;
  static bool lastRedDecreaseState = HIGH;
  static bool lastBlueIncreaseState = HIGH;
  static bool lastBlueDecreaseState = HIGH;

  bool redIncreaseState = digitalRead(redIncreasePin);
  bool redDecreaseState = digitalRead(redDecreasePin);
  bool blueIncreaseState = digitalRead(blueIncreasePin);
  bool blueDecreaseState = digitalRead(blueDecreasePin);

  if (redIncreaseState == LOW && lastRedIncreaseState == HIGH) {
    redScore++;
    updateLCD();
  } else if (redDecreaseState == LOW && lastRedDecreaseState == HIGH) {
    redScore--;
    updateLCD();
  } else if (blueIncreaseState == LOW && lastBlueIncreaseState == HIGH) {
    blueScore++;
    updateLCD();
  } else if (blueDecreaseState == LOW && lastBlueDecreaseState == HIGH) {
    blueScore--;
    updateLCD();
  } 
  lastRedIncreaseState = redIncreaseState;
  lastRedDecreaseState = redDecreaseState;
  lastBlueIncreaseState = blueIncreaseState;
  lastBlueDecreaseState = blueDecreaseState;
  delay(50);
}

/*
  Function: updateLCD()

  Description: Updates the LCD display with the current scores for the red and blue teams. 
               Checks for winning conditions and displays the winning message if a team has won.

  Parameters: None

  Returns: None
*/
void updateLCD() {
  lcd.clear();
   
  // Check for winning condition
  if (totalRedScore >= 21) {
    lcd.setCursor(0, 0);
    lcd.print("RED TEAM WON!!");
    return; // Exit early to avoid overwriting the win message
  }
  if (totalBlueScore >= 21) {
    lcd.setCursor(0, 0);
    lcd.print("BLUE TEAM WON!!");
    return;
  }
   
  // Default score display if no winner
  lcd.setCursor(3, 0);
  lcd.print(totalRedScore);
  lcd.setCursor(12, 0);
  lcd.print(totalBlueScore);
  lcd.setCursor(3, 1);
  lcd.print((roundState == ACTIVE || roundState == MANUAL || roundState == CONFIRM) ? redScore : 0);
  lcd.setCursor(12, 1);
  lcd.print((roundState == ACTIVE || roundState == MANUAL || roundState == CONFIRM) ? blueScore : 0);
}

/*
  Function: updateLEDs()

  Description: Updates the state of the LEDs based on the current round state. 
               Activates the appropriate LED for active or manual scoring modes.

  Parameters: None

  Returns: None
*/
void updateLEDs() {
  if (roundState == ACTIVE) {
    digitalWrite(activeScoringLEDPin, HIGH);
    digitalWrite(manualScoringLEDPin, LOW);
  } else if (roundState == MANUAL) {
    digitalWrite(activeScoringLEDPin, LOW);
    digitalWrite(manualScoringLEDPin, HIGH);
  } else {
    digitalWrite(activeScoringLEDPin, LOW);
    digitalWrite(manualScoringLEDPin, LOW);
  }
}

/*
  Function: processInput()

  Description: Processes Bluetoothinput strings to update scores for red and blue teams. 
               Handles score increments, decrements, and total score calculations. 
               Updates the round state and LEDs accordingly.

  Parameters:
    - input (String): The input string containing the color, sign, and score.

  Returns: None
*/
void processInput(String input) {

  if (input == "Reset"){
    redScore = 0;
    blueScore = 0;
    totalBlueScore = 0;
    totalRedScore = 0;
    RedReg = 0; 
    BlueReg = 0; 
    OnRed = 0;
    OnBlue = 0;
    roundState = StartingState;
    updateLEDs();
  } 

  if (ReadingState == true){
    if (input.indexOf(',') != -1) {
      // Full scoreboard packet: totalRed,totalBlue,roundRed,roundBlue.
      int comma1 = input.indexOf(',');
      int comma2 = input.indexOf(',', comma1 + 1);

      String redRoundStr = input.substring(0, comma1);
      String blueRoundStr = input.substring(comma1 + 1, comma2);

      redScore = redRoundStr.toInt();
      blueScore = blueRoundStr.toInt();
    }

    if (input == "Switch"){
      int scoreDifference = abs(redScore - blueScore);
      if (redScore > blueScore) {
        totalRedScore += scoreDifference;
      } else if (blueScore > redScore) {
        totalBlueScore += scoreDifference;
      } 
      redScore = 0;
      blueScore = 0;
      roundState = ACTIVE;
      updateLEDs();
    }
  }

  updateLCD();
}

/*
  Function: GetColors()

  Description: Detects the color of objects using sensor readings and updates the LED states accordingly. 
               Adjusts scores based on detected colors and updates the LCD display.

  Parameters: None

  Returns: None
*/
void GetColors() {
  static bool redDetected_A = false;  // Flag to track when a red bag has been detected
  static bool blueDetected_A = false;  // Flag to track when a blue bag has been detected
  static bool greenDetected_A = false;  // Flag to track when a green background has been detected

  //static unsigned long lastDetectionTime = 0; // Time of last detection

  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);                                          
  Red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  Serial.println("Red");        
  Serial.println(Red);
  delay(20);  
  digitalWrite(s2, HIGH);
  digitalWrite(s3, HIGH);  
  Green = pulseIn(out,  digitalRead(out) == HIGH ? LOW : HIGH);
  Serial.println("Green");
  Serial.println(Green);
  delay(20);  
  digitalWrite(s2, LOW);
  digitalWrite(s3, HIGH);        
  Blue = pulseIn(out, digitalRead(out) == HIGH ? LOW  : HIGH);
  Serial.println("Blue");
  Serial.println(Blue);
  Serial.println("____________________________________________________");
  delay(20);  

  //Sensor Comparison
  if ((Red<Blue) && (Red<Green)){ //Red Threshold
    redDetected_A = true;
  } else if ((Green<=Blue) || (abs(Green-Blue)<18)){
    if((Green<=Red) || (abs(Green-Red)<20)){ //Green Threshold
      greenDetected_A = true;
    }
  } else if ((Blue<Red) && (Blue<Green)){ //Blue Threshold
    blueDetected_A = true;
  } else {
    redDetected_A = false;
    greenDetected_A = false;
    blueDetected_A = false;
  }

  while(greenDetected_A == true){
    if(redDetected_A == true){
      redScore = redScore + 3;
    }
    else if(blueDetected_A == true){
      blueScore = blueScore + 3;
    }

    redDetected_A = false;
    greenDetected_A = false;
    blueDetected_A = false;
  }

  updateLCD();
}

/*
  Function: OnBoard()

  Description: Processes the Pixy camera blocks to detect objects on the board. 
               Updates the scores for red and blue teams based on object positions and sizes. 
               Adjusts the scores and updates the LCD display accordingly.

  Parameters: None

  Returns: None
*/
void OnBoard() {
  OnRed = 0;
  OnBlue = 0;
  pixy.ccc.getBlocks();
    for (int i = 0; i < pixy.ccc.numBlocks; i++){
      int x = pixy.ccc.blocks[i].m_x;
      int y = pixy.ccc.blocks[i].m_y;
      int s = pixy.ccc.blocks[i].m_signature;
      if (x > 90 && x < 215 && y > 5 && y < 195){ //This is supposed to be the board area
      if (x > 90 && x < 215 && y > 85 && y < 195){ //Area of close section
          if (s == redSig){
            if ((pixy.ccc.blocks[i].m_width > 70 && pixy.ccc.blocks[i].m_height > 65) || (pixy.ccc.blocks[i].m_width > 115 || pixy.ccc.blocks[i].m_height > 90)){ //Area of 4 bags
              OnRed += 4;
              //Serial.print("Close Red 4 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 75 || pixy.ccc.blocks[i].m_height > 70){ //Area of 3 bags
              OnRed += 3;
              //Serial.print("Close Red 3 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 50 || pixy.ccc.blocks[i].m_height > 50){ //Area of 2 bags
              OnRed += 2;
              //Serial.print("Close Red 2 called \n");
              //delay(1000);
            }
            else{ //else is one bag
              OnRed += 1;
              //Serial.print("Close Red 1 called \n");
              //delay(1000);
            }
          }
          if (s == blueSig){
            if ((pixy.ccc.blocks[i].m_width > 70 && pixy.ccc.blocks[i].m_height > 65) || (pixy.ccc.blocks[i].m_width > 115 || pixy.ccc.blocks[i].m_height > 90)){ //Area of 4 bags
              OnBlue += 4;
              //Serial.print("Close blue 4 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 75 || pixy.ccc.blocks[i].m_height > 70){ //Area of 3 bags
              OnBlue += 3;
              //Serial.print("Close blue 3 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 50 || pixy.ccc.blocks[i].m_height > 50){
              OnBlue += 2;
              //Serial.print("Close blue 2 called \n");
              //delay(1000);
            }
            else{
              OnBlue += 1;
              //Serial.print("Close blue 1 called \n");
              //delay(1000);
            }
          }
        }
        else if (x > 90 && x < 210 && y > 50 && y < 85){
          if (s == redSig){
            if ((pixy.ccc.blocks[i].m_width > 50 && pixy.ccc.blocks[i].m_height > 45) || (pixy.ccc.blocks[i].m_width > 85 || pixy.ccc.blocks[i].m_height > 65)){
              OnRed += 4;
              //Serial.print("Middle Red 4 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 50 || pixy.ccc.blocks[i].m_height > 50){
              OnRed += 3;
              //Serial.print("Middle Red 3 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 35 || pixy.ccc.blocks[i].m_height > 35){
              OnRed += 2;
              //Serial.print("Middle Red 2 called \n");
              //delay(1000);
            }
            else{
              OnRed += 1;
              //Serial.print("Middle Red 1 called \n");
              //delay(1000);
            }
          }
          if (s == blueSig){
            if ((pixy.ccc.blocks[i].m_width > 50 && pixy.ccc.blocks[i].m_height > 45) || (pixy.ccc.blocks[i].m_width > 85 || pixy.ccc.blocks[i].m_height > 65)){
              OnBlue += 4;
              //Serial.print("Middle blue 4 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 50 || pixy.ccc.blocks[i].m_height > 50){
              OnBlue += 3;
              //Serial.print("Middle blue 3 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 35 || pixy.ccc.blocks[i].m_height > 35){
              OnBlue += 2;
              //Serial.print("Middle blue 2 called \n");
              //delay(1000);
            }
            else{
              OnBlue += 1;
              //Serial.print("Middle blue 1 called \n");
              //delay(1000);
            }
          }
        }
        else if (x > 80 && x < 195 && y > 5 && y < 50){
          if (s == redSig){
            if ((pixy.ccc.blocks[i].m_width > 40 && pixy.ccc.blocks[i].m_height > 35) || (pixy.ccc.blocks[i].m_width > 70 || pixy.ccc.blocks[i].m_height > 50)){
              OnRed += 4;
              //Serial.print("Far Red 4 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 40 || pixy.ccc.blocks[i].m_height > 40){
              OnRed += 3;
              //Serial.print("Far Red 3 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 30 || pixy.ccc.blocks[i].m_height > 30){
              OnRed += 2;
              //Serial.print("Far Red 2 called \n");
              //delay(1000);
            }
            else{
              OnRed += 1;
              //Serial.print("Far Red 1 called \n");
              //delay(1000);
            }
          }
          if (s == blueSig){
            if ((pixy.ccc.blocks[i].m_width > 40 && pixy.ccc.blocks[i].m_height > 35) || (pixy.ccc.blocks[i].m_width > 70 || pixy.ccc.blocks[i].m_height > 50)){
              OnBlue += 4;
              //Serial.print("Far blue 4 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 40 || pixy.ccc.blocks[i].m_height > 40){
              OnBlue += 3;
              //Serial.print("Far blue 3 called \n");
              //delay(1000);
            }
            else if (pixy.ccc.blocks[i].m_width > 30 || pixy.ccc.blocks[i].m_height > 30){
              OnBlue += 2;
              //Serial.print("Far blue 2 called \n");
              //delay(1000);
            }
            else{
              OnBlue += 1;
              //Serial.print("Far blue 1 called \n");
              //delay(1000);
            }
          }
        }
      }
    }
  if (OnRed > 4){
    OnRed = 4;
  }
  if (OnBlue > 4){
    OnBlue = 4;
  }

  if (OnRed != RedReg){
    redScore += OnRed - RedReg;
    if (OnRed < RedReg){
      int difference = abs(OnRed - RedReg);
      //sending = "R-" + String(difference) + ".";
    } else if (OnRed > RedReg){
      int difference = abs(OnRed - RedReg);
      //sending = "R+" + String(difference) + ".";
    }
    RedReg = OnRed;
  }

  if (OnBlue != BlueReg){
    blueScore += OnBlue - BlueReg;
    if (OnBlue < BlueReg){
      int difference = abs(OnBlue - BlueReg);
      //sending = "B-" + String(difference) + ".";
    } else if (OnBlue > BlueReg){
      int difference = abs(OnBlue - BlueReg);
      //sending = "B+" + String(difference) + ".";
    }
    BlueReg = OnBlue;
  }

  updateLCD();

  // Serial.print("Red Score: ");
  // Serial.print(OnRed);
  // Serial.print(", Blue Score: ");
  // Serial.print(OnBlue);
  // Serial.print('\n');
}
