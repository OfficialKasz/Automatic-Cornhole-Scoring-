/*
File:        Bluetooth_Code_v1.ino
Author:      Lukasz Janik
Date:        2025-03-26
Description: 
  The code essentially sets up a Bluetooth communication system where the state of a button controls the state of two LEDs, 
    and data is exchanged between the Bluetooth module and the Serial monitor.
Version:     1.0
*/

#include <SoftwareSerial.h>

// Define Bluetooth Pins
SoftwareSerial Bluetooth(2, 3); //RX, TX

#define Button 8
#define ledPinA 9
#define ledPinB 10

//Button States
int buttonState = 0;
int state = 0;

//Bluetooth & Serial Characters
char sending = ' ';
char receiving = ' ';


void setup() {
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  pinMode(ledPinA, OUTPUT);
  pinMode(ledPinB, OUTPUT);
  pinMode(Button, INPUT);

  // Toggle LEDs off initally
  digitalWrite(ledPinA, LOW);
  digitalWrite(ledPinB, LOW);

  // Start both Bluetooth and Serial
  Serial.begin(38400);
  Serial.println("\n Serial Print is Ready");
  Bluetooth.begin(38400);
  Serial.println("\n Bluetooth Started");
}

void loop() {

  // We listen to Bluetooth with this
  if (Bluetooth.available()){
    receiving = Bluetooth.read();
    Serial.write(receiving);
  }

  // Button Logic
  buttonState = digitalRead(Button);
  if (buttonState == LOW) {
    sending = '1';
    state = 1;
  } else {
    sending = '0';
    state = 0;
  }  

  // Reading Bluetooth Data
  if (receiving == '1') {
    digitalWrite(ledPinB, HIGH); // LED ON
    receiving = '0';
  }
  else if (receiving == '0') {
    digitalWrite(ledPinB, LOW); // LED ON
    receiving = '0';
  }

  // Indicator to see if button works correctly
  if (state == 1) {
    digitalWrite(ledPinA, HIGH); // LED ON
    state = 1;
  }
  else if (state == 0) {
    digitalWrite(ledPinA, LOW); // LED ON
    state = 0;
  }

  // We write to the Bluetooth with this
  if (Serial.available()){;}
    Serial.print(sending); // Sends '1' to the master to turn on LED
    Bluetooth.print(sending);

  // Cleans up Serial Monitor
  Serial.print("\n");
  delay(10);
}
