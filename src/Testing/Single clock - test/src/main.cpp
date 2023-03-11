/*
  X12.017 Quad Driver Test without library
  Drive the motors CLOCKWISE and COUNTER-CLOCKWISE in 180 deg
  at constant speed.
 */

#include <Arduino.h>

const int STEP_A = 19; // Driver p28 for f(scx)A
const int DIR_A = 18;  // Driver p27 for CW/CCW
const int STEP_D = 25; // pin for f(scx)
const int DIR_D = 26;  // pin for CW/CCW

const int LED = 2;
const int RESET = 4;  // pin for RESET
const int DELAY = 700; // μs between steps
const int ANGLE = 180; // of 360 available
const int RANGE = ANGLE * 12; // one step = 1/12 deg, which means RANGE * 12
int steps = 0;
bool forward = true;   //true(CW), false(CCW)

// pull RESET low to reset, high for normal operation.

void setup() {
  Serial.begin(115200);
  pinMode(STEP_A, OUTPUT);
  pinMode(DIR_A, OUTPUT);
  pinMode(STEP_D, OUTPUT);
  pinMode(DIR_D, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(STEP_A, LOW);
  digitalWrite(STEP_D, LOW);
  digitalWrite(RESET, LOW);
  digitalWrite(LED, HIGH);
  delay(1);  // keep reset low min 1ms
  digitalWrite(RESET, HIGH);
}

// The motor steps on rising edge of STEP
// The step line must be held low for at least 450ns
// which is so fast we probably don't need a delay,
// put in a short delay for good measure.
void loop() {
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_D, LOW);

  digitalWrite(STEP_A, LOW);
  delayMicroseconds(1);
  digitalWrite(STEP_A, HIGH);
  delayMicroseconds(DELAY);

  digitalWrite(STEP_D, LOW);
  delayMicroseconds(1);
  digitalWrite(STEP_D, HIGH);
  delayMicroseconds(DELAY);
}

/*
Conclusion
The output is okay, the stepper motors is really smooth.
However multiple steppers would be difficult because of calling blocking code.
An solution could be to build my own stepper library.
For a single clock this could work tho.
*/