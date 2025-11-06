#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

void calibrate() {
  //increase or decrease pulseLength manually 
  pwm.setPWM(2, 0,250);
}

void setup() {
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(50);
}

void loop() {
  calibrate();
  delay(1000);
}
