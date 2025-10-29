#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

void calibrate() {
  //increase or decrease pulseLength manually 
  pwm.setPWM(0, 0, 570);
}

void setup() {
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(50);
  randomSeed(A3);
}

void loop() {
  calibrate();
  delay(1000);
}
