#include <StepperMotorMovement.hpp>


StepperMotorMovement::StepperMotorMovement(Time time, HelperFunction helper) : time_(time), helper_(helper)
{
    motorHours_ = AccelStepper(AccelStepper::DRIVER, STEP_A_, DIR_A_);
    motorMinutes_ = AccelStepper(AccelStepper::DRIVER, STEP_D_, DIR_D_);

    motorMinutes_.setPinsInverted(true);
    motorHours_.setPinsInverted(true);
    xSemaphoreMin = xSemaphoreCreateBinary();
    xSemaphoreHour = xSemaphoreCreateBinary();
}

StepperMotorMovement::~StepperMotorMovement()
{
}

void StepperMotorMovement::SetupResetPin()
{
    pinMode(RESET_, OUTPUT);
    digitalWrite(RESET_, LOW);
    delay(1);  // keep reset low min 1ms
    digitalWrite(RESET_, HIGH);
}

void StepperMotorMovement::StartMinuteMotorThread(long minuteMotorPos)
{  
  minuteMotorPos_ = minuteMotorPos;
  //Start thread
  xTaskCreate
  (
    this->StartMinuteTask,             //function name
    "Move minute motor threaded",     //task name
    40000,                             //stack size
    (void *)this,            //task parameters
    1,                                //task priority
    &handleMinTask                    //task handle
  );
}

void StepperMotorMovement::StartMinuteTask(void* _this)
{
  static_cast<StepperMotorMovement*>(_this)->MoveMinuteMotorInDeg();
}

void StepperMotorMovement::MoveMinuteMotorInDeg()
{
  Serial.print("Moving minute into position: ");
  Serial.print(this->minuteMotorPos_);
  Serial.println(" deg");

  for(;;) // loop until destination
  {
    if (motorMinutes_.currentPosition() == 0 || motorMinutes_.currentPosition() == 360*12)
    {
    long setPos = (motorMinutes_.currentPosition() == 0)? 360 : 0;
    motorMinutes_.setCurrentPosition(setPos*12);
    motorMinutes_.run(); // Remember lost step by changing pos +1
    }

    motorMinutes_.moveTo((this->minuteMotorPos_)*12);  // Set moveTo pos of hour motor

    // check that pos is within 360 deg
    if (this->minuteMotorPos_ >= 0 && this->minuteMotorPos_ <= 360)  
    {
        if (motorMinutes_.distanceToGo() != 0)  
        {
            motorMinutes_.run();
        }
        if (motorMinutes_.distanceToGo() == 0) 
        {
            // Serial.print("Minute motor(");
            // Serial.print(motorMinutes_.speed());
            // Serial.print(", ");
            // Serial.print(motorMinutes_.currentPosition());
            // Serial.print(") - ");
            // Serial.print("COMPLETED!");
            // Serial.println("");
            // Serial.println("\n");
            xSemaphoreGive(xSemaphoreMin);
            // vTaskDelete(handleHourTask);
            // handleHourTask = NULL;
            vTaskSuspend(handleMinTask); //suspend thread
        }
    }
  }
}

void StepperMotorMovement::StartHourMotorThread(long hourMotorPos)
{
  hourMotorPos_ = hourMotorPos;
  //Start thread
  xTaskCreate
  (
    this->StartHourTask,             //function name
    "Move hour motor threaded",     //task name
    40000,                           //stack size
    (void*)this,            //task parameters
    1,                              //task priority
    &handleHourTask                 //task handle
  );
}

void StepperMotorMovement::StartHourTask(void* _this)
{
  static_cast<StepperMotorMovement*>(_this)->MoveHourMotorInDeg();
}

void StepperMotorMovement::MoveHourMotorInDeg()
{
  Serial.print("Moving hours into position: ");
  Serial.print(this->hourMotorPos_);
  Serial.println(" deg");

  for(;;) // loop until destination
  {
    if (this->motorHours_.currentPosition() == 0 || this->motorHours_.currentPosition() == 360*12) 
    {
        long setPos = (this->motorHours_.currentPosition() == 0)? 360 : 0;
        this->motorHours_.setCurrentPosition(setPos*12);
        this->motorHours_.run(); // Remember lost step by changing pos +1
    }

    this->motorHours_.moveTo(this->hourMotorPos_*12);  // Set moveTo pos of hour motor

    // check that pos is within 360 deg
    if (this->hourMotorPos_ >= 0 && this->hourMotorPos_ <= 360)  
    {
        if (this->motorHours_.distanceToGo() != 0)  
        {
            this->motorHours_.run();
        }
        if (this->motorHours_.distanceToGo() == 0) 
        {
            // Serial.print("Hour motor(");
            // Serial.print(this->motorHours_.speed());
            // Serial.print(", ");
            // Serial.print(this->motorHours_.currentPosition());
            // Serial.print(") - ");
            // Serial.print("COMPLETED!");
            // Serial.println("\n");
            xSemaphoreGive(xSemaphoreHour); //release work
            //vTaskDelete(handleHourTask);
            //handleHourTask = NULL;
            vTaskSuspend(handleHourTask); //suspend thread
        }
    }
  }
}

void StepperMotorMovement::Homing()
{
  time_.RTC_.read(time_.tm_); //read time from RTC

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
  
  //MoveBothMotorsInDeg(minuteDegrees_, hourDegrees_); 
    MoveBothMotorsInDeg(315, 45);
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
    StartHourMotorThread(hourStepInDegrees_);
    if (hourStepInDegrees_ == 360)
    {
      hourStepInDegrees_ = 0;
    }
  }
  if (WhichMotor == "MinuteMotor")
  {
    minuteStepInDegrees_ = minuteStepInDegrees_ + 1;
    StartMinuteMotorThread(minuteStepInDegrees_);
    if (minuteStepInDegrees_ == 360)
    {
      minuteStepInDegrees_ = 0;
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

    StartMinuteMotorThread(minuteMotorPos);
    StartHourMotorThread(hourMotorPos);

    Serial.print("Minute Task Stack -> ");
    Serial.println(uxTaskGetStackHighWaterMark(handleMinTask));
    Serial.print("Hour Task Stack -> ");
    Serial.println(uxTaskGetStackHighWaterMark(handleHourTask));
}

void StepperMotorMovement::AnimationOne()
{
    bool animationDone = false;
    int i = 0;
    //Setup
    motorMinutes_.setCurrentPosition(180*12);   // sets the current position
    motorMinutes_.setMaxSpeed(500);            // Set Max Speed of Stepper (Slower to get better accuracy)
    motorMinutes_.setAcceleration(1000);        // Set Acceleration of Stepper
    
    motorHours_.setCurrentPosition(180*12);   // sets the current position as 180 degress in steps for now
    motorHours_.setMaxSpeed(500);            // Set Max Speed of Stepper (Slower to get better accuracy)
    motorHours_.setAcceleration(1000);        // Set Acceleration of Stepper

    int minuteAnimationArr[] = {
        315,
        225,
        315,
        225,
        180};

    int hourAnimationArr[] = {        
        45,
        135,
        45,
        135,
        180};

    //Start Animation
    Serial.println("Animation started");
    xSemaphoreGive(xSemaphoreMin);
    xSemaphoreGive(xSemaphoreHour);
    while(animationDone != true)
    { 
        if (xSemaphoreTake(xSemaphoreMin, 0) == pdPASS && xSemaphoreTake(xSemaphoreHour, 0) == pdPASS) // Check if the semaphore is available
        {   
            if (i == 2) //sizeof(hourAnimationArr) / sizeof(int)
            {
                animationDone = true;
            }
            Serial.print("Semaphores taken, i -> ");
            Serial.println(i);           
            vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for some time before calling vTask again
            if (i == 0)
            {
                StartMinuteMotorThread(minuteAnimationArr[i]);
                StartHourMotorThread(hourAnimationArr[i]);
            }
            xSemaphoreGive(xSemaphoreMin); // Release the semaphores
            xSemaphoreGive(xSemaphoreHour);
            vTaskResume(handleMinTask); // Resume the suspended task
            vTaskResume(handleMinTask); // Resume the suspended task
            i++;
        }
    } //block code until task is done
    Serial.println("Animation done"); 
}


