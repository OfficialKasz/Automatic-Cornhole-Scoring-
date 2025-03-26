//Sensor A pinout
#define s0 8        
#define s1 9
#define s2 10
#define s3 11
#define out_a 12

//Sensor B's unique pin
#define out_b 13

//Sensor A LEDs
#define R_LED_A 2
#define G_LED_A 3
#define B_LED_A 4
#define R_LED_B 5
#define G_LED_B 6
#define B_LED_B 7

int  Red_A=0, Blue_A=0, Green_A=0;  //RGB values for Sensor A
int  Red_B=0, Blue_B=0, Green_B=0;  //RGB values for Sensor B

void setup() 
{
   //Sensor pins
   pinMode(s0,OUTPUT);   
   pinMode(s1,OUTPUT);
   pinMode(s2,OUTPUT);
   pinMode(s3,OUTPUT);
   pinMode(out_a,INPUT);
   pinMode(out_b,INPUT);

   //LED Pins
   pinMode(R_LED_A,OUTPUT);
   pinMode(G_LED_A,OUTPUT);
   pinMode(B_LED_A,OUTPUT);
   pinMode(R_LED_B,OUTPUT);
   pinMode(G_LED_B,OUTPUT);
   pinMode(B_LED_B,OUTPUT);

   //Starting 
   Serial.begin(9600);   
   digitalWrite(s0,HIGH); 
   digitalWrite(s1,HIGH);  
   
}

void loop(){
  
  GetColors();


  if ((Red_A<Blue_A) && (Red_A<Green_A)){ //Red Threshold
    digitalWrite(R_LED_A, HIGH);
    digitalWrite(G_LED_A, LOW);
    digitalWrite(B_LED_A, LOW);
  } else if (Green_A<Blue_A){
      if((Green_A<=Red_A) || (abs(Green_A-Red_A)<20)){ //Green Threshold
        digitalWrite(R_LED_A, LOW);
        digitalWrite(G_LED_A, HIGH);
        digitalWrite(B_LED_A, LOW);
      }
  } else if ((Blue_A<Red_A) && (Blue_A<Green_A)){ //Blue Threshold
    digitalWrite(R_LED_A, LOW);
    digitalWrite(G_LED_A, LOW);
    digitalWrite(B_LED_A, HIGH);
  } else {
    digitalWrite(R_LED_A, HIGH);
    digitalWrite(G_LED_A, HIGH);
    digitalWrite(B_LED_A, HIGH);
  }


  if ((Red_B<Blue_B) && (Red_B<Green_B)){ //Red Threshold
    digitalWrite(R_LED_B, HIGH);
    digitalWrite(G_LED_B, LOW);
    digitalWrite(B_LED_B, LOW);
  } else if (Green_B<Blue_B){
      if((Green_B<=Red_B) || (abs(Green_B-Red_B)<20)){ //Green Threshold
        digitalWrite(R_LED_B, LOW);
        digitalWrite(G_LED_B, HIGH);
        digitalWrite(B_LED_B, LOW);
      }
  } else if ((Blue_B<Red_B) && (Blue_B<Green_B)){ //Blue Threshold
    digitalWrite(R_LED_B, LOW);
    digitalWrite(G_LED_B, LOW);
    digitalWrite(B_LED_B, HIGH);
  } else {
    digitalWrite(R_LED_B, HIGH);
    digitalWrite(G_LED_B, HIGH);
    digitalWrite(B_LED_B, HIGH);
  }

  //delay(2000);
     
}

void GetScore(){
  /*
  when the color switches off of green
    if sensor A = sensor B
      add 3 to the color that was seen

    if sensor A != sensor B
      add 3 to both scoreboards  
  */
}


//Sensor A Color readings 
void GetColors()  
{    
  digitalWrite(s2, LOW); 
  digitalWrite(s3, LOW);                                           
  Red_A = pulseIn(out_a, digitalRead(out_a) == HIGH ? LOW : HIGH);
  Red_B = pulseIn(out_b, digitalRead(out_b) == HIGH ? LOW : HIGH);        
  Serial.println(Red_A);
  Serial.println(Red_B);
  delay(20);  
  digitalWrite(s2, HIGH); 
  digitalWrite(s3, HIGH);   
  Green_A = pulseIn(out_a,  digitalRead(out_a) == HIGH ? LOW : HIGH);
  Green_B = pulseIn(out_b,  digitalRead(out_b) == HIGH ? LOW : HIGH);
  Serial.println(Green_A);
  Serial.println(Green_B);
  delay(20);  
  digitalWrite(s2, LOW); 
  digitalWrite(s3, HIGH);         
  Blue_A = pulseIn(out_a, digitalRead(out_a) == HIGH ? LOW  : HIGH);
  Blue_B = pulseIn(out_b, digitalRead(out_b) == HIGH ? LOW  : HIGH);
  Serial.println(Blue_A);
  Serial.println(Blue_B);
  Serial.println("____________________________________________________");
  delay(20);  
}
