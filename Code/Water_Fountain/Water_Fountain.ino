#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVO_DRIVER_ADDRESS 0x40
#define NUM_SERVOS 10
#define SERVO_MIN 80
#define SERVO_MAX 550
#define POT0 A0
#define POT1 A1
#define POT2 A2

int ppValue1, ppValue2, ppValue3; 
int aRange = 10;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(SERVO_DRIVER_ADDRESS);

void _init(){
  pwm.begin();
  pwm.setPWMFreq(50);
}

void pot1Response(){
  int npValue = analogRead(A0);
  Serial.print("New pot1 value: ");
  Serial.println(npValue);

  //if the new pot value falls outside this range
  if(npValue < (ppValue1 - aRange) ||npValue > (ppValue1 + aRange)){
    //controller servo motor 0 - 3 
    int angle1  = random(270);
    int angle2  = random(270);
    int angle3  = random(270);
    int angle4  = random(270);


    Serial.print("Motor 1 angle: ");
    Serial.print(angle1);
    Serial.print(" Motor 2 angle: ");
    Serial.print(angle2);
    Serial.print(" Motor 4 angle: ");
    Serial.print(angle3);
    Serial.print(" Motor 7 angle: ");
    Serial.println(angle4);


    int pulseLength1 = map(angle1,0,270,SERVO_MIN,SERVO_MAX);
    int pulseLength2 = map(angle2,0,270,SERVO_MIN,SERVO_MAX);
    int pulseLength3 = map(angle3,0,270,SERVO_MIN,SERVO_MAX);
    int pulseLength4 = map(angle4,0,270,SERVO_MIN,SERVO_MAX);

    pwm.setPWM(0,0,pulseLength1);
    pwm.setPWM(1,0,pulseLength2);
    pwm.setPWM(3,0,pulseLength3);
    pwm.setPWM(6,0,pulseLength4);

    ppValue1 = npValue;
    
  }
}
void pot2Response(){
  int npValue = analogRead(A1);
  Serial.print("New pot2 value: ");
  Serial.println(npValue);

  //if the new pot value falls outside this range
  if(npValue < (ppValue2 - aRange) ||npValue > (ppValue2 + aRange)){
    //controller servo motor 4 - 7 
    int angle1  = random(270);
    int angle2  = random(270);
    int angle3  = random(270);


    Serial.print("Motor 3 angle: ");
    Serial.print(angle1);
    Serial.print(" Motor 5 angle: ");
    Serial.print(angle2);
    Serial.print(" Motor 8 angle: ");
    Serial.println(angle3);


    int pulseLength1 = map(angle1,0,270,SERVO_MIN,SERVO_MAX);
    int pulseLength2 = map(angle2,0,270,SERVO_MIN,SERVO_MAX);
    int pulseLength3 = map(angle3,0,270,SERVO_MIN,SERVO_MAX);

    pwm.setPWM(2,0,pulseLength1);
    pwm.setPWM(4,0,pulseLength2);
    pwm.setPWM(7,0,pulseLength3);

    ppValue2 = npValue;
    
  }
}
void pot3Response(){
  int npValue = analogRead(A2);
  Serial.print("New pot3 value: ");
  Serial.println(npValue);

  //if the new pot value falls outside this range
  if(npValue < (ppValue3 - aRange) ||npValue > (ppValue3 + aRange)){
    //controller servo motor 0 -3 
    int angle1  = random(270);
    int angle2  = random(270);
    int angle3  = random(270);


    Serial.print("Motor 6 angle: ");
    Serial.print(angle1);
    Serial.print(" Motor 9 angle: ");
    Serial.print(angle2);
    Serial.print(" Motor 10 angle: ");
    Serial.println(angle3);


    int pulseLength1 = map(angle1,0,270,SERVO_MIN,SERVO_MAX);
    int pulseLength2 = map(angle2,0,270,SERVO_MIN,SERVO_MAX);
    int pulseLength3 = map(angle3,0,270,SERVO_MIN,SERVO_MAX);

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
  Serial.begin(9600);
  _init();
  randomSeed(analogRead(A3));
}
void loop(){
  run();
  delay(1000);
}
