/*
File:        One_Color_Sensor.ino
Author:      Lukasz Janik
Date:        2025-03-26
Description: 
  The code sets up a color sensor system where the RGB values are read from the sensor, 
     and the corresponding LED is turned on based on the detected color. 
     The RGB values are also printed to the Serial monitor.
Version:     1.0
*/

//Sensor A pinout
#define s0 8        
#define s1 9
#define s2 10
#define s3 11
#define out 12

//Sensor A LEDs
#define R_LED 2
#define G_LED 3
#define B_LED 4

int  Red=0, Blue=0, Green=0;  //RGB values for Sensor A

void setup() 
{
   //Sensor pins
   pinMode(s0,OUTPUT);   
   pinMode(s1,OUTPUT);
   pinMode(s2,OUTPUT);
   pinMode(s3,OUTPUT);
   pinMode(out,INPUT);

   //LED Pins
   pinMode(R_LED,OUTPUT);
   pinMode(G_LED,OUTPUT);
   pinMode(B_LED,OUTPUT);

   //Starting 
   Serial.begin(9600);   
   digitalWrite(s0,HIGH); 
   digitalWrite(s1,HIGH);  
   
}

void loop(){
  
  GetColors();

  // Comparison of Colors to see which LED to turn on
  if ((Red<Blue) && (Red<Green)){ 
    digitalWrite(R_LED, HIGH);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, LOW);
  } else if ((Blue<Red) && (Blue<Green)){ 
    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, HIGH);
  } else {
    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, HIGH);
    digitalWrite(B_LED, LOW);
  }

  delay(2000);
     
}

//Sensor A Color readings 
void GetColors()  
{    
  digitalWrite(s2, LOW); 
  digitalWrite(s3, LOW);                                           
  Red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  Serial.println(Red);
  delay(20);  
  digitalWrite(s2, HIGH); 
  digitalWrite(s3, HIGH);   
  Green = pulseIn(out,  digitalRead(out) == HIGH ? LOW : HIGH);
  Serial.println(Green);
  delay(20);  
  digitalWrite(s2, LOW); 
  digitalWrite(s3, HIGH);         
  Blue = pulseIn(out, digitalRead(out) == HIGH ? LOW  : HIGH);
  Serial.println(Blue);
  Serial.println("____________________________________________________");
  delay(20);  
}
