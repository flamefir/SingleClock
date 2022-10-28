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

//Wifi class object 
WifiConnect wifi;

//NTP server request time data
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Time object
tmElements_t tm;

//Time class
Time timeRtcNtp(RTC, tm, timeClient);

//Helper functions
HelperFunction helper(timeRtcNtp.RTC_, timeRtcNtp.tm_);

//Motor class
StepperMotorMovement motor(timeRtcNtp, helper);

// Setup ISR env
volatile bool interrupt = false;
int tenSecondCounter = 0;
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
ICACHE_RAM_ATTR
#endif

void sqwHandler()
{
  interrupt = true;
}

void setup() {

  delay(500);
  Serial.begin(115200);

  // Initialize pins
  pinMode(ledPin, OUTPUT);
  pinMode(motor.RESET_, OUTPUT);
  digitalWrite(motor.RESET_, LOW);
  delay(1);  // keep reset low min 1ms
  digitalWrite(motor.RESET_, HIGH);
  pinMode(INT_PIN, INPUT_PULLUP);                                       // ISR
  attachInterrupt(digitalPinToInterrupt(INT_PIN), sqwHandler, FALLING); // ISR

  wifi.BeginWiFiConnection();

  timeRtcNtp.SetupNTP();
  timeRtcNtp.SetupRTCInterrupt();
  timeRtcNtp.SetTimeInRTC();

  delay(5000);
  motor.Homing();
}

void loop() {

  if (interrupt)
  {  
    timeRtcNtp.RTC_.read(timeRtcNtp.tm_);

    if (helper.GetDigitInt(timeRtcNtp.tm_.Second, 0) == 0)
    {
      helper.PrintTimeFromRTC(timeRtcNtp.timeParse_, timeRtcNtp.RTCConfig_);
      motor.Step("MinuteMotor");
    }

    if ((timeRtcNtp.tm_.Minute % 2) == 0 && timeRtcNtp.tm_.Second == 0) //even ten's number
    {
      helper.PrintTimeFromRTC(timeRtcNtp.timeParse_, timeRtcNtp.RTCConfig_);
      motor.Step("HourMotor");
    }
    interrupt = false; // clear flag
  }
}