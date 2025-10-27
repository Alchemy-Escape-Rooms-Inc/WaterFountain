#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

void setup(){
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(1000);
}

void loop(){
  //pwm.setPWM(0,1024,3072);
}

