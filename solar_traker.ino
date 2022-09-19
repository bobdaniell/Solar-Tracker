#include <SparkFun_TB6612.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT11
#define DHTPIN 12
#define PWMA 7
#define AIN2 6
#define AIN1 5
#define STBY 48
#define BIN1 4
#define BIN2 3
#define PWMB 2

DHT dht(DHTPIN,DHTTYPE);

float t = 0.0;
float h = 0.0;
  
const int analogIn = A4;
int mVperAmp = 66 ; // use 100 for 20A Module and 66 for 30A Module
int analogValue = 0; //analog reading 
int ACSoffset = 2500; 
double Voltage = 0;
double Amps = 0;
float voltage = 0.0;
float power = 0.0;

const float ref_voltage = 5;
unsigned long R1 = 7500;
unsigned long R2 = 30000;
const int analogPin = A5;
const int inputResolution = 1023;

const int offsetA = 1;
const int offsetB = 1;
 

Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

char mycommand;
bool execute = false;

void setup() { 
 
  pinMode(8, INPUT);   //pin8 for switch1
  pinMode(9, INPUT);   // pin9 for switch2
  pinMode(13, OUTPUT); //pin13 for LED
  dht.begin(); 
  Serial.begin(115200); 
  Serial1.begin(115200);
   
} 

void loop() { 
 
 if( Serial1.available()>0){
  
   char mycommand = Serial1.read();
   
   if ( mycommand == '9'){
    
    execute = true;
   }
  if ( mycommand == '0'){
   
    execute = false;
   }
 }
   
  if(execute == true)
  {
   ManualMode();
  }
  else{
    AutoMode();
  }
  
  Temp_Hum();
 voltageMeasurement();
 currentMeasurement();
  sendData();
  delay(400);
}
 
void currentMeasurement(){
  
  analogValue = analogRead(analogIn);
 Voltage = (analogValue / 1024.0) * 5000; 
 Amps = ((Voltage - ACSoffset) / mVperAmp);

 if(Amps < 0 ){
  Amps =  0; 
 }
}

void voltageMeasurement(){
  
  int A5Value = analogRead(analogPin);
  float voltage_sensed = A5Value * (ref_voltage / (float)inputResolution);   
  voltage = voltage_sensed * ( 1 + ( (float)R2 /  (float)R1) );
}

void Temp_Hum(){

  t = dht.readTemperature();
  h = dht.readHumidity (); 
}

void sendData(){
     
 power = voltage * Amps ; 
 
 if (power < 0 ){
  
  power = 0;
 }
 
 Serial1.print('#'); 
 Serial1.print (t); 
 Serial1.print('+'); 
 Serial1.print (h);
 Serial1.print('+');
 Serial1.print (voltage); 
 Serial1.print('+'); 
 Serial1.print (Amps,3); 
 Serial1.print('+'); 
 Serial1.print (power);
 Serial1.println(' '); 
 Serial1.print('~'); //used as an end of transmission character  
 Serial1.println();
 }


 void ManualMode(){
   digitalWrite(13, LOW);
   
   if (Serial1.available()>0){
    
     mycommand = Serial1.read();
    
   if(digitalRead(8) == HIGH  && digitalRead(9) == HIGH ){ 
    
    switch(mycommand){
        case '1': 
            motor2.drive(65,300);
            motor2.brake();
            break;
    
        case '7':
            motor2.drive(-65,300);
            motor2.brake();
            break;
            
        
    }
   }
   
   if(  digitalRead(8) == HIGH && digitalRead(9) == LOW){
         
     switch(mycommand){
        case '1':
            motor2.drive(65,300);
            motor2.brake();
            break;
            
     }
   }
   if( digitalRead(8) == LOW && digitalRead(9) == HIGH){
    
        switch(mycommand){
        case '7':
            motor2.drive(-65,300);
            motor2.brake();
            break; 
      }
   }
      switch(mycommand){
        case '3':
             motor1.drive(-90,400);
             motor1.brake();
             break;
             
         case '4':
            motor1.drive(-90,100);
            break;
            
        case '5':
            motor1.drive(90,400);
            motor1.brake();
            break;
            
         case '6':
            motor1.drive(90,100);
            break;
       }
    }
 }
  
void AutoMode(){
  digitalWrite(13, HIGH);
    
    int topright;
    int topleft;
    int downright;
    int downleft;
    int threshold_value= 50;
     
  topleft = analogRead(A0);
  topright = analogRead(A3);
  downleft = analogRead(A1);
  downright = analogRead(A2);
  
   ////////////average//////////
   int avgtop = ( topleft + topright)/2;
   int avgdown = ( downleft + downright)/2;
   int avgleft = ( topleft + downleft)/2;
   int avgright = ( topright + downright)/2;

   //// diference////

   int diff_z_axis = avgtop - avgdown;
   int diff_x_axis = avgright - avgleft;

   ///left-right
   if( abs(diff_x_axis) >= threshold_value){
    if (diff_x_axis > 0){
      motor1.drive(90,50); // miscare dreapta
    }
    
    if (diff_x_axis < 0){
      motor1.drive(-90, 50);   
    }
   }
    if( abs(diff_x_axis) <= threshold_value){
      motor1.brake();
    }
    /////up-down
    
   if(abs(diff_z_axis) >= threshold_value){
    if (diff_z_axis > 0){
      motor2.drive (65,50); 
    }
    if(diff_z_axis < 0){
      motor2.drive(-65,50); 
    }
   }
   
    if(abs(diff_z_axis) <= threshold_value){
     motor2.stop();
    }
    
    if ( digitalRead(8) == LOW || digitalRead(9) == LOW){
      motor2.brake();
    }
  }
