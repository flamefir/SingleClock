#include <Wire.h>
#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <elapsedMillis.h>
#include <ErriezDS1307.h>

#define DS1307_CTRL_ID 0x68
#define INT_PIN 4
#define ledPin 2

//Stepper motor A
const int STEP_A = 23; // Driver p28 for f(scx)A
const int DIR_A = 19;  // Driver p27 for CW/CCW
//Stepper motor D
const int STEP_D = 18; // Driver p17 for f(scx)A
const int DIR_D = 5;   // Driver p16 for CW/CCW


const int RESET = 15;  // pin for RESET
const int TOTALSTEPS = 360 * 12; // of 360 available
int secondsInSteps = 0;
int minutesInSteps = 1;
int hoursInSteps = 1;


AccelStepper motor_hours(AccelStepper::DRIVER, STEP_A, DIR_A);
AccelStepper motor_minutes(AccelStepper::DRIVER, STEP_D, DIR_D);
MultiStepper steppers; //up to 10 steppers
elapsedMillis printTime;

// Stepper Travel Variables
int move_finished_hour = 1;     // Used to check if hour pointer is completed
int move_finished_min = 1;      // Used to check if minute pointer is completed
long initial_homing = -1;       // Used to Home Stepper at startup


// Create DS1307 RTC object
ErriezDS1307 rtc;

// SQW interrupt flag must be volatile
volatile bool sqwInterrupt = false;

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
ICACHE_RAM_ATTR
#endif
void sqwHandler()
{
  // Set global interrupt flag
  sqwInterrupt = true;
}

// init functions
void MoveSteppersInDeg(long motorAPos, long motorDPos);
void HomeHourPtr();

void setup() {
    // Initialize serial port
  delay(500);
  Serial.begin(115200);
  while (!Serial) {
      ;
  }

  // Initialize pins
  pinMode(ledPin, OUTPUT);
  pinMode(RESET, OUTPUT);

  digitalWrite(RESET, LOW);
  delay(1);  // keep reset low min 1ms
  digitalWrite(RESET, HIGH);

  // Initialize TWI
  Wire.begin();
  Wire.setClock(400000);

  // Attach to INT0 interrupt falling edge
  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), sqwHandler, FALLING);

  // Initialize RTC
  while (!rtc.begin()) {
      Serial.println(F("RTC not found"));
      delay(3000);
  }

  // Enable RTC clock
  if (!rtc.isRunning()) {
      rtc.clockEnable();
  }

  // Set 1Hz square wave out:
  rtc.setSquareWave(SquareWave1Hz);

  //Setup and home hour pointer
  HomeHourPtr();

  //Init steppers
  steppers.addStepper(motor_hours);
  steppers.addStepper(motor_minutes);
}

int counter = 0;

void loop() {
  // if (sqwInterrupt) //ISR interrupt
  // {
  //   if (counter == 60)
  //   {
  //     secondsInSteps = 0;
  //     counter = 0;
  //   }
  //   secondsInSteps = secondsInSteps + 6;
  //   MoveSteppersInDeg(secondsInSteps, secondsInSteps);
  //   Serial.print("hello from ISR: counter = ");
  //   Serial.print(counter);
  //   Serial.print(", steps = ");
  //   Serial.println(secondsInSteps);
  //   counter++;
  //   sqwInterrupt = false;
  // }

  MoveSteppersInDeg(0, 270);
  delay(1000);
}

void MoveSteppersInDeg(long minuteMotorPos, long hourMotorPos)
{
  move_finished_hour = 0;  
  move_finished_min = 0;

  Serial.print("Moving minute motor into position: ");
  Serial.print(minuteMotorPos);
  Serial.println(" deg");
  Serial.print("Moving hours into position: ");
  Serial.print(hourMotorPos);
  Serial.println(" deg");

  //Check if minute and hour pointer should go from 0-360 or 360-0 for continous rotation
  if (motor_minutes.currentPosition() == 0 || motor_minutes.currentPosition() == 360*12)
  {
    long setPos = (motor_minutes.currentPosition() == 0)? 360 : 0;
    motor_minutes.setCurrentPosition(setPos*12);
    motor_minutes.run(); // Remember lost step by changing pos +1
  }
  if (motor_hours.currentPosition() == 0 || motor_hours.currentPosition() == 360*12)
  {
    long setPos = (motor_hours.currentPosition() == 0)? 360 : 0;
    motor_hours.setCurrentPosition(setPos*12);
    motor_hours.run(); // Remember lost step by changing pos +1
  }


  while (move_finished_hour == 0 && move_finished_min == 0)
  {
    motor_minutes.moveTo(minuteMotorPos*12);  // Set moveTo pos of minute motor
    motor_hours.moveTo(hourMotorPos*12);  // Set moveTo pos of hour motor

    // check that pos is within 360 deg
    if ((minuteMotorPos >= 0 && minuteMotorPos <= 360) && (hourMotorPos >= 0 && hourMotorPos <= 360))  
    {

      // Check if minute and hour motor has reached desired position if not -> step
      if (motor_minutes.distanceToGo() != 0)  
      {
        motor_minutes.run();  
      }
      if (motor_hours.distanceToGo() != 0)  
      {
        motor_hours.run();
      }

      // check if move is completed then reset
      if (move_finished_min == 0 && motor_minutes.distanceToGo() == 0 && move_finished_min != 1) 
      {
        Serial.print("Minute motor(");
        Serial.print(motor_minutes.speed());
        Serial.print(", ");
        Serial.print(motor_minutes.currentPosition());
        Serial.println(")");
        Serial.println("COMPLETED!");

        move_finished_min = 1;  // Reset motor
      }
      if (move_finished_hour == 0 && motor_hours.distanceToGo() == 0 && move_finished_hour != 1) 
      {
        Serial.print("Hour motor(");
        Serial.print(motor_hours.speed());
        Serial.print(", ");
        Serial.print(motor_hours.currentPosition());
        Serial.println(")");
        Serial.println("COMPLETED!");

        move_finished_hour = 1; // Reset motor
      }
    }
  }
}

void HomeHourPtr()
{
  motor_minutes.setPinsInverted(true);
  //Start homing procedure of stepper motor at startup
  Serial.print("Stepper is Homing . . . . . . . . . . . ");  
  
  //For the time being we set the homing to 6 o'clock
  motor_minutes.setCurrentPosition(180*12);  // sets the current position as 180 degress in steps for now
  motor_minutes.setMaxSpeed(100);            // Set Max Speed of Stepper (Slower to get better accuracy)
  motor_minutes.setAcceleration(100);        // Set Acceleration of Stepper

  //For the time being we set the homing to 6 o'clock
  motor_hours.setCurrentPosition(180*12);  // sets the current position as 180 degress in steps for now
  motor_hours.setMaxSpeed(100);            // Set Max Speed of Stepper (Slower to get better accuracy)
  motor_hours.setAcceleration(100);        // Set Acceleration of Stepper

  initial_homing = 1;

  // while (motor1.currentPosition() != time.seconds())   // Make the Stepper move CW until it reaches correct time
  // {              
  //   motor1.moveTo(initial_homing);  
  //   motor1.run();
  //   initial_homing++;
  //   delay(5);
  // }

  Serial.println("Homing Completed");
  Serial.println("");
  motor_minutes.setMaxSpeed(1000.0);        // Set Max Speed of Stepper
  motor_minutes.setAcceleration(1000.0);    // Set Acceleration of Stepper

  motor_hours.setMaxSpeed(1000.0);        // Set Max Speed of Stepper
  motor_hours.setAcceleration(1000.0);    // Set Acceleration of Stepper
}