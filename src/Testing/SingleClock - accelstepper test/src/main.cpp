#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <elapsedMillis.h>

const int STEP_A = 23; // Driver p28 for f(scx)A
const int DIR_A = 22;  // Driver p27 for CW/CCW
const int STEP_D = 18; // pin for f(scx)
const int DIR_D = 5;  // pin for CW/CCW

const int LED = 2;
const int RESET = 4;  // pin for RESET
const int DELAY = 700; // μs between steps
const int STEPS = 360 * 12; // of 360 available

int steps = 0;
bool forward = true;   //true(CW), false(CCW)

AccelStepper motor1(AccelStepper::DRIVER,STEP_A, DIR_A);
AccelStepper motor2(AccelStepper::DRIVER,STEP_D, DIR_D);
MultiStepper steppers; //up to 10 steppers
elapsedMillis printTime;


void setup() {
  Serial.begin(9600);
  pinMode(RESET, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  digitalWrite(RESET, LOW);
  delay(1);  // keep reset low min 1ms
  digitalWrite(RESET, HIGH);

  // Then give them to MultiStepper to manage
  steppers.addStepper(motor1);
  steppers.addStepper(motor2);

  // Configure each stepper
  motor1.setMaxSpeed(1000);
  motor1.setAcceleration(150);
  motor2.setMaxSpeed(1000);
  motor2.setAcceleration(50);

  motor1.moveTo(STEPS/2);
  
}

void loop() {

  if(printTime >= 1000)
  {
    printTime = 0;
    float m1Speed = motor1.speed();
    float m2Speed = motor2.speed();

    Serial.print("Motor1(");
    Serial.print(m1Speed);
    Serial.print(", ");
    Serial.print(motor1.currentPosition());
    Serial.print(")\t Motor2(");
    Serial.print(m2Speed);
    Serial.print(", ");
    Serial.print(motor2.currentPosition());
    Serial.println(")");
  }
  if(!motor1.run())
  {
    motor1.moveTo(-motor1.currentPosition());
  }
  //motor1.run();
}