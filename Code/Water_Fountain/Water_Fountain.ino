#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVO_DRIVER_ADDRESS 0x40
#define NUM_SERVOS 10
#define POT0 A0
#define POT1 A1
#define POT2 A2

int ppValue1, ppValue2, ppValue3; 
int aRange = 10;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(SERVO_DRIVER_ADDRESS);


void pot1Response(){
  int npValue = analogRead(A0);
  //Serial.print("New pot1 value: ");
  //Serial.println(npValue);

  //if the new pot value falls outside this range
  if(npValue < (ppValue1 - aRange) ||npValue > (ppValue1 + aRange)){
    //controller servo motor 0 - 3 
    //Serial.print("Motor angle: ");


    int pulseLength1 = map(npValue,0,1023,85,290);
    int pulseLength2 = map(npValue,0,1023,350,155);
    int pulseLength3 = map(npValue,0,1023,250,85);
    int pulseLength4 = map(npValue,0,1023,250,550);

    pwm.setPWM(0,0,pulseLength1);
    pwm.setPWM(1,0,pulseLength2);
    pwm.setPWM(3,0,pulseLength3);
    pwm.setPWM(6,0,pulseLength4);

    ppValue1 = npValue;
    
  }
}
void pot2Response(){
  int npValue = analogRead(A1);
  //Serial.print("New pot2 value: ");
  //Serial.println(npValue);

  //if the new pot value falls outside this range
  if(npValue < (ppValue2 - aRange) ||npValue > (ppValue2 + aRange)){
    //controller servo motor 4 - 7 
    
    //Serial.print("Motor  angle: ");
    //Serial.print(npValue);

    int pulseLength1 = map(npValue,0,1023,90,250);
    int pulseLength2 = map(npValue,0,1023,108,330);
    int pulseLength3 = map(npValue,0,1023,250,410);

    pwm.setPWM(2,0,pulseLength1);
    pwm.setPWM(4,0,pulseLength2);
    pwm.setPWM(7,0,pulseLength3);

    ppValue2 = npValue;
    
  }
}
void pot3Response(){
  int npValue = analogRead(A2);
  //Serial.print("New pot3 value: ");
  //Serial.println(npValue);

  //if the new pot value falls outside this range
  if(npValue < (ppValue3 - aRange) ||npValue > (ppValue3 + aRange)){
    //controller servo motor 0 -3 
    
    //Serial.print("Motor angle: ");
    //Serial.print(npValue);
  
    int pulseLength1 = map(npValue,0,1023,210,450);
    int pulseLength2 = map(npValue,0,1023,150,410);
    int pulseLength3 = map(npValue,0,1023,410,550);

    pwm.setPWM(5,0,pulseLength1);
    pwm.setPWM(8,0,pulseLength2);
    pwm.setPWM(9,0,pulseLength3);

    ppValue3 = npValue;
    
  }
}
void run(){
  pot1Response();
  pot2Response();
  pot3Response();
}



void setup(){
 // Serial.begin(9600);
  //Serial.println("Program started.");
  pwm.begin();
  pwm.setPWMFreq(50);
}
void loop(){
  run();
  delay(100);
}
