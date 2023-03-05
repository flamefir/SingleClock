/*
This iteration is official ocean code clean up

Define classes:
  - StepperMotorMovement -> all motor functions and variables
  - Time -> Handles all setup and time related stuff with NTP server and RTC
  - WiFiConnect -> a simple wifi class with all wifi functions and variables + NTP server connection
  - HelperFunctions -> helper function for debug messages, and make some code less messy
*/

#include <StepperMotorMovement.hpp>
#include <Wire.h>
#include <BluetoothSerial.h>
#include <TimeLib.h>
#include <elapsedMillis.h>

#define DS1307_CTRL_ID 0x68
#define INT_PIN 4

#define ledPin 2

// Time object
tmElements_t tm;

// Time class
Time myTimer(RTC, tm);

// Helper functions
HelperFunction helper(myTimer.RTC_, myTimer.tm_);

// Motor class
StepperMotorMovement motor(myTimer, helper);

// Thread class
TaskHandle_t task1Handle = NULL;

// bluetooth
BluetoothSerial ESP_BT;

int counter1 = 0;
int counter = 0;

// Setup ISR env
volatile bool interrupt = false;
// volatile bool interrupt_bt1 = false;
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
ICACHE_RAM_ATTR
#endif

void sqwHandler()
{
  interrupt = true;
}

void setup()
{
  delay(500);
  Serial.begin(115200);
  motor.SetupResetPin();
  // Initialize pins
  pinMode(ledPin, OUTPUT);

  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), sqwHandler, FALLING);

  ESP_BT.begin("Clock", true);
  
  // TODO get bluetooth read data from app
  myTimer.SetupRTCInterrupt();
  myTimer.SetTimeInRTC();

  delay(5000);
  motor.Homing();
}

void loop()
{
  if (interrupt)
  {
    myTimer.RTC_.read(myTimer.tm_);

    if (helper.GetDigitInt(myTimer.tm_.Second, 0) == 0)
    {
      helper.PrintTimeFromRTC(myTimer.timeParse_, myTimer.RTCConfig_);
      motor.Step("MinuteMotor");
    }

    if ((myTimer.tm_.Minute % 2) == 0 && myTimer.tm_.Second == 0)
    {
      helper.PrintTimeFromRTC(myTimer.timeParse_, myTimer.RTCConfig_);
      motor.Step("HourMotor");
    }
    interrupt = false; // clear flag
  }
}
