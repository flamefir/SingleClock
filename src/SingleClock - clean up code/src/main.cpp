/*
This iteration is official ocean code clean up

Define classes:
  - StepperMotorMovement -> all motor functions and variables
  - Time -> Handles all setup and time related stuff with NTP server and RTC
  - WiFiConnect -> a simple wifi class with all wifi functions and variables + NTP server connection
  - HelperFunctions -> helper function for debug messages, and make some code less messy
*/

#include <StepperMotorMovement.hpp>
#include <WifiConnect.hpp>
#include <Wire.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <elapsedMillis.h>
#include "ESPAsyncWebServer.h"

#define DS1307_CTRL_ID 0x68
#define INT_PIN 4

#define ledPin 2

// Wifi class object
WifiConnect wifi;

// NTP server request time data
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Time object
tmElements_t tm;

// Time class
Time myTimer(RTC, tm, timeClient);

// Helper functions
HelperFunction helper(myTimer.RTC_, myTimer.tm_);

// Motor class
StepperMotorMovement motor(myTimer, helper);

// Thread class
TaskHandle_t task1Handle = NULL;

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
  // pinMode(BT1_PIN, INPUT);
  // attachInterrupt(BT1_PIN, sqwHandler_bt1, FALLING);

  wifi.ConnectToWiFi();

  myTimer.SetupNTP();
  myTimer.SetupRTCInterrupt();
  myTimer.SetTimeInRTC();

  delay(5000);
  motor.Homing();

  wifi.DisconnectFromWiFi();
}

void loop()
{
  if (interrupt)
  {
    myTimer.RTC_.read(myTimer.tm_);
    myTimer.UpdateDST(myTimer.tm_);

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
