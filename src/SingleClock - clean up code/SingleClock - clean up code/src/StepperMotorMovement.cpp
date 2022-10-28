#include <StepperMotorMovement.hpp>


StepperMotorMovement::StepperMotorMovement(Time time, HelperFunction helper) : time_(time), helper_(helper)
{
    motorHours_ = AccelStepper(AccelStepper::DRIVER, STEP_A_, DIR_A_);
    motorMinutes_ = AccelStepper(AccelStepper::DRIVER, STEP_D_, DIR_D_);
}

StepperMotorMovement::~StepperMotorMovement()
{
}

void StepperMotorMovement::MoveMinuteMotorInDeg(long minuteMotorPos)
{
  Serial.print("Moving minute into position: ");
  Serial.print(minuteMotorPos);
  Serial.println(" deg");

  if (motorMinutes_.currentPosition() == 0 || motorMinutes_.currentPosition() == 360*12)
  {
    long setPos = (motorMinutes_.currentPosition() == 0)? 360 : 0;
    motorMinutes_.setCurrentPosition(setPos*12);
    motorMinutes_.run(); // Remember lost step by changing pos +1
  }

  motorsFinished_ = 0;
  while (motorsFinished_ < 1) // if both motors are done, then break while loop
  {
    motorMinutes_.moveTo(minuteMotorPos*12);  // Set moveTo pos of hour motor

    // check that pos is within 360 deg
    if (minuteMotorPos >= 0 && minuteMotorPos <= 360)  
    {
      if (motorMinutes_.distanceToGo() != 0)  
      {
        motorMinutes_.run();
      }
      if (motorMinutes_.distanceToGo() == 0) 
      {
        Serial.print("Minute motor(");
        Serial.print(motorMinutes_.speed());
        Serial.print(", ");
        Serial.print(motorMinutes_.currentPosition());
        Serial.print(") - ");
        Serial.print("COMPLETED!");
        Serial.println("");
        Serial.println("\n");
        motorsFinished_ = motorsFinished_ + 1; // increment to break while loop
      }
    }
  }
}

void StepperMotorMovement::MoveHourMotorInDeg(long hourMotorPos)
{
  Serial.print("Moving hours into position: ");
  Serial.print(hourMotorPos);
  Serial.println(" deg");


  motorsFinished_ = 0;
  while (motorsFinished_ < 1) // if both motors are done, then break while loop
  {

    if (motorHours_.currentPosition() == 0 || motorHours_.currentPosition() == 360*12) 
    {
      long setPos = (motorHours_.currentPosition() == 0)? 360 : 0;
      motorHours_.setCurrentPosition(setPos*12);
      motorHours_.run(); // Remember lost step by changing pos +1
    }
    motorHours_.moveTo(hourMotorPos*12);  // Set moveTo pos of hour motor

    // check that pos is within 360 deg
    if (hourMotorPos >= 0 && hourMotorPos <= 360)  
    {
      if (motorHours_.distanceToGo() != 0)  
      {
        motorHours_.run();
      }
      if (motorHours_.distanceToGo() == 0) 
      {
        Serial.print("Hour motor(");
        Serial.print(motorHours_.speed());
        Serial.print(", ");
        Serial.print(motorHours_.currentPosition());
        Serial.print(") - ");
        Serial.print("COMPLETED!");
        Serial.println("\n");
        motorsFinished_ = motorsFinished_ + 1; // increment to break while loop
      }
    }
  }
}

void StepperMotorMovement::MoveBothMotorsInDeg(long minuteMotorPos, long hourMotorPos)
{
  Serial.print("Moving minute motor into position: ");
  Serial.print(minuteMotorPos);
  Serial.println(" deg");
  Serial.print("Moving hours into position: ");
  Serial.print(hourMotorPos);
  Serial.println(" deg");
  Serial.println("");

  //Check if minute and hour pointer should go from 0-360 or 360-0 for continous rotation
  if (motorMinutes_.currentPosition() == 0 || motorMinutes_.currentPosition() == 360*12)
  {
    long setPos = (motorMinutes_.currentPosition() == 0)? 360 : 0;
    motorMinutes_.setCurrentPosition(setPos*12);
    motorMinutes_.run(); // Remember lost step by changing pos +1
  }
  if (motorHours_.currentPosition() == 0 || motorHours_.currentPosition() == 360*12)
  {
    long setPos = (motorHours_.currentPosition() == 0)? 360 : 0;
    motorHours_.setCurrentPosition(setPos*12);
    motorHours_.run(); // Remember lost step by changing pos +1
  }

  motorsFinished_ = 0;
  while (motorsFinished_ < 2) // if both motors are done, then break while loop
  {
    motorMinutes_.moveTo(minuteMotorPos*12);  // Set moveTo pos of minute motor
    motorHours_.moveTo(hourMotorPos*12);  // Set moveTo pos of hour motor

    // check that pos is within 360 deg
    if ((minuteMotorPos >= 0 && minuteMotorPos <= 360) && (hourMotorPos >= 0 && hourMotorPos <= 360))  
    {

      // if minute and hour motor has not reached desired position -> step
      if (motorMinutes_.distanceToGo() != 0)  
      {
        motorMinutes_.run(); 

      }
      if (motorMinutes_.distanceToGo() == 0) 
      {
        Serial.print("Minute motor(");
        Serial.print(motorMinutes_.speed());
        Serial.print(", ");
        Serial.print(motorMinutes_.currentPosition());
        Serial.print(") - ");
        Serial.print("COMPLETED!");
        Serial.println("");
        motorsFinished_ = motorsFinished_ + 1;  // increment to break while loop
      } 

      if (motorHours_.distanceToGo() != 0)  
      {
        motorHours_.run();
      }
      if (motorHours_.distanceToGo() == 0) 
      {
        Serial.print("Hour motor(");
        Serial.print(motorHours_.speed());
        Serial.print(", ");
        Serial.print(motorHours_.currentPosition());
        Serial.print(") - ");
        Serial.print("COMPLETED!");
        Serial.println("");
        motorsFinished_ = motorsFinished_ + 1; // increment to break while loop
      }
    }
  }
}

void StepperMotorMovement::Homing()
{
  time_.RTC_.read(time_.tm_); //read time from RTC

  motorMinutes_.setPinsInverted(false);
  motorHours_.setPinsInverted(true);

  //Start homing procedure of stepper motor at startup
  Serial.print("Stepper is Homing to (");  
  Serial.print(time_.tm_.Hour);
  Serial.print(" : ");
  Serial.print(time_.tm_.Minute);  
  Serial.println(") . . . . . . . . . . .  \n"); 

  motorMinutes_.setCurrentPosition(180*12);   // sets the current position as 180 degress in steps for now
  motorMinutes_.setMaxSpeed(1000);            // Set Max Speed of Stepper (Slower to get better accuracy)
  motorMinutes_.setAcceleration(1000);        // Set Acceleration of Stepper

  motorHours_.setCurrentPosition(180*12);   // sets the current position as 180 degress in steps for now
  motorHours_.setMaxSpeed(1000);            // Set Max Speed of Stepper (Slower to get better accuracy)
  motorHours_.setAcceleration(1000);        // Set Acceleration of Stepper

  initial_homing_ = 1;
  
  // Hours -> pointer is dependent on the minutes etc. 30 min = 50 % of 30 degrees = 15 degrees  smooth -> |0.5 degree each minute|
  hourDegrees_ = ConvHourToRotation(time_.tm_.Hour) + (time_.tm_.Minute/2);
  hourStepInDegrees_ = hourDegrees_;
  // Minutes -> pointer is dependent on the seconds etc. 30 sec = 50 % of 12 degrees = 6 degrees smooth -> |1 degree for each 10s|
  minuteDegrees_ = ConvMinToRotation(time_.tm_.Minute) + helper_.GetDigitInt(time_.tm_.Second, 1);
  minuteStepInDegrees_ = minuteDegrees_;
  
  MoveBothMotorsInDeg(minuteDegrees_, hourDegrees_); 

  Serial.println("Homing Completed");
}

long StepperMotorMovement::ConvHourToRotation(int hours)
{
  //Convert to 12 hour format
  if(hours == 0 && hours != 12)
  {
    hours = 0;
  }
  else if(hours == 12 && hours != 0)
  {
    hours = 0;
  }
  else if(hours < 12 && hours != 0)
  {
    hours = hours;
  }  
  else if(hours > 12 && hours != 0)
  { 
    hours = hours-12; 
  }

  return hours*30; // 1 hour equals 30 degrees
}

long StepperMotorMovement::ConvMinToRotation(int minutes)
{
  return minutes*6; // 1 minute equals 6 degrees
}

// "MinuteMotor" or "HourMotor"
void StepperMotorMovement::Step(String WhichMotor)
{
  if (WhichMotor == "HourMotor")
  {
    hourStepInDegrees_ = hourStepInDegrees_ + 1;
    MoveHourMotorInDeg(hourStepInDegrees_);
    if (hourStepInDegrees_ == 360)
    {
      hourStepInDegrees_ = 0;
    }
  }
  if (WhichMotor == "MinuteMotor")
  {
    minuteStepInDegrees_ = minuteStepInDegrees_ + 1;
    MoveMinuteMotorInDeg(minuteStepInDegrees_);
    if (minuteStepInDegrees_ == 360)
    {
      minuteStepInDegrees_ = 0;
    }
  }
}