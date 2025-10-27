#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVO_DRIVER_ADDRESS 0x40
#define NUM_SERVOS 10
#define POT0 A0
#define POT1 A1
#define POT2 A2

//---------DATA STRUCTURES----------
struct servoP {
  byte channel;
  byte position = 0, correctPosition;
};
//---------GLOBAL VARIABLES---------

servoP servos[NUM_SERVOS];
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(SERVO_DRIVER_ADDRESS);

//---------FUNCTION PROTOTYPES------

void _init();
void gpiosInit();
void servosInit();
void calibrateServos();
void run();
void setServoPosition(byte position, servoP & servo);
void updateServoPosition(const servoP & servo);

//---------SETUP FUNCTION----------- 
void setup(){
  Serial.begin(9600);
  _init();
  randomSeed(analogRead(A3));
}

//---------MAIN LOOP-----------------
void loop(){
  run();
}

void _init(){
  gpiosInit();
  servosInit();
}
void gpiosInit(){
  pinMode(POT0,INPUT);
  pinMode(POT1,INPUT);
  pinMode(POT2,INPUT);
}
void servosInit(){
  //set all the servos to their initial positions
  pwm.begin();
  pwm.setPWMFreq(1000);
  for(byte i = 0; i < NUM_SERVOS; i++)
    servos[i].channel = i;
}

void calibrateServos(){
  //pulselength = map(degrees, 0, 180, SERVOMIN, SERVOMAX);
}
void run(){

}

void setServoPosition(byte position, servoP & servo){
  servo.position = position;
}
void updateServoPosition(const servoP & servo){
  
}
