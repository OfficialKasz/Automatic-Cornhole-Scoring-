#include <LiquidCrystal.h>

// Define button pins
const int redIncreasePin = 7;
const int redDecreasePin = 8;
const int blueIncreasePin = 9;
const int blueDecreasePin = 10;
const int endRoundPin = 6;

// Score variables
int redScore = 0, blueScore = 0;
int totalRedScore = 0, totalBlueScore = 0;

// Round state variables
enum RoundState { ACTIVE, MANUAL, CONFIRM };
RoundState roundState = ACTIVE;

// LCD Initialization
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//Color Sensors shared pins
#define s0 22
#define s1 23
#define s2 24
#define s3 25

//Sensor A's Unique Pinout
#define out_a 26

// Sensor A LEDs
#define R_LED_A 32
#define G_LED_A 34
#define B_LED_A 36

int  Red_A=0, Blue_A=0, Green_A=0;  //RGB values for Sensor A

void setup() {
    lcd.begin(16, 2);
    pinMode(redIncreasePin, INPUT_PULLUP);
    pinMode(redDecreasePin, INPUT_PULLUP);
    pinMode(blueIncreasePin, INPUT_PULLUP);
    pinMode(blueDecreasePin, INPUT_PULLUP);
    pinMode(endRoundPin, INPUT_PULLUP);

    //Sensor pins
    pinMode(s0,OUTPUT);   
    pinMode(s1,OUTPUT);
    pinMode(s2,OUTPUT);
    pinMode(s3,OUTPUT);
    pinMode(out_a,INPUT);

    //LED Pins
    pinMode(R_LED_A,OUTPUT);
    pinMode(G_LED_A,OUTPUT);
    pinMode(B_LED_A,OUTPUT);

    Serial.begin(9600); 
    digitalWrite(s0, HIGH);
    digitalWrite(s1, HIGH);

    updateLCD();
}

void loop() {
    static bool lastEndRoundState = HIGH;
    bool endRoundState = digitalRead(endRoundPin);

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
            redScore = 0;
            blueScore = 0;
            roundState = CONFIRM;
        } else if (roundState == CONFIRM) {
            roundState = ACTIVE;
        }
        updateLCD();
    }
    lastEndRoundState = endRoundState;

    if (roundState == ACTIVE) {
        GetColors();
    } else if (roundState == MANUAL) {
        handleScoring();
    }
}

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
    }
    if (redDecreaseState == LOW && lastRedDecreaseState == HIGH) {
        redScore--;
        updateLCD();
    }
    if (blueIncreaseState == LOW && lastBlueIncreaseState == HIGH) {
        blueScore++;
        updateLCD();
    }
    if (blueDecreaseState == LOW && lastBlueDecreaseState == HIGH) {
        blueScore--;
        updateLCD();
    }

    lastRedIncreaseState = redIncreaseState;
    lastRedDecreaseState = redDecreaseState;
    lastBlueIncreaseState = blueIncreaseState;
    lastBlueDecreaseState = blueDecreaseState;
    delay(50);
}

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

void GetColors() {
    static bool redDetected_A = false;  // Flag to track when a red bag has been detected
    static bool blueDetected_A = false;  // Flag to track when a blue bag has been detected
    static bool greenDetected_A = false;  // Flag to track when a green background has been detected

    //static unsigned long lastDetectionTime = 0; // Time of last detection

    digitalWrite(s2, LOW); 
    digitalWrite(s3, LOW);                                           
    Red_A = pulseIn(out_a, digitalRead(out_a) == HIGH ? LOW : HIGH);
    Serial.println("Red");        
    Serial.println(Red_A);
    delay(20);  
    digitalWrite(s2, HIGH); 
    digitalWrite(s3, HIGH);   
    Green_A = pulseIn(out_a,  digitalRead(out_a) == HIGH ? LOW : HIGH);
    Serial.println("Green"); 
    Serial.println(Green_A);
    delay(20);  
    digitalWrite(s2, LOW); 
    digitalWrite(s3, HIGH);         
    Blue_A = pulseIn(out_a, digitalRead(out_a) == HIGH ? LOW  : HIGH);
    Serial.println("Blue"); 
    Serial.println(Blue_A);
    Serial.println("____________________________________________________");
    delay(20);  

    //Sensor A's Comparison
    if ((Red_A<Blue_A) && (Red_A<Green_A)){ //Red Threshold
      digitalWrite(R_LED_A, HIGH);
      digitalWrite(G_LED_A, LOW);
      digitalWrite(B_LED_A, LOW);
      redDetected_A = true;
    } else if (Green_A<Blue_A){
      if((Green_A<=Red_A) || (abs(Green_A-Red_A)<30)){ //Green Threshold
        digitalWrite(R_LED_A, LOW);
        digitalWrite(G_LED_A, HIGH);
        digitalWrite(B_LED_A, LOW);
        greenDetected_A = true;
      }
    } else if ((Blue_A<Red_A) && (Blue_A<Green_A)){ //Blue Threshold
      digitalWrite(R_LED_A, LOW);
      digitalWrite(G_LED_A, LOW);
      digitalWrite(B_LED_A, HIGH);
      blueDetected_A = true;
    } else {
      digitalWrite(R_LED_A, HIGH);
      digitalWrite(G_LED_A, HIGH);
      digitalWrite(B_LED_A, HIGH);
      redDetected_A = false;
      greenDetected_A = false;
      blueDetected_A = false;
    }
    
    //delay(2000);
    
    while(greenDetected_A == true){
      if(redDetected_A == true){
        redScore = redScore + 3;
      }
      if(blueDetected_A == true){
        blueScore = blueScore + 3;
      }

      redDetected_A = false;
      greenDetected_A = false;
      blueDetected_A = false;
    }

    updateLCD();
}

