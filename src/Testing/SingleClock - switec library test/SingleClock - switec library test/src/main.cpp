#include <Arduino.h>
#include <SwitecX12.h> //to use with x12.17 motor driver

const int STEP_A = 19; // Driver p28 for A f(scx)A
const int DIR_A = 18;  // Driver p27 for A CW/CCW
const int STEP_D = 25; // Driver p17 for D f(scx)A
const int DIR_D = 26;  // Driver p16 for D CW/CCW

const int RESET = 4;  // pin for RESET
const int STEPS = 315 * 12; // of 360 available

SwitecX12 motor1(STEPS, STEP_A, DIR_A);

void setup() {
  digitalWrite(RESET, HIGH);
  Serial.begin(9600);
}

void loop() {
  digitalWrite(2, HIGH);
  motor1.update();
}

/*
Conclusion
Could never figure out how make this work.
*/