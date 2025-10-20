#define NUM_SERVOS 10
#define POT0 A0
#define POT1 A1
#define POT2 A2

//---------DATA STRUCTURES----------
struct servoP {
  byte position = 0, correctPosition;
};
//---------GLOBAL VARIABLES---------

servoP servos[NUM_SERVOS];

//---------FUNCTION PROTOTYPES------

void _init();
void gpios_init();
void servos_init();
void run();



//---------SETUP FUNCTION----------- 
void setup(){
  _init();
}

//---------MAIN LOOP-----------------
void loop(){
  run();
}

void _init(){
  gpios_init();
  servos_init();
}
void gpios_init(){
  pinMode(POT0,INPUT);
  pinMode(POT1,INPUT);
  pinMode(POT2,INPUT);
}
void servos_init(){
  //set all the servos to their initial positions
}

void run(){

}
