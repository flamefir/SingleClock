/*
This test will home the hour and minute pointers to the exact time
This is done by:
  - Connecting to the internet
  - Setting the RTC time
  - Convert time into 12 hour format, then into 360 degrees format
  - Run motors until destination is reached
*/

#include "WiFi.h"
#include <Wire.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <DS1307RTC.h>
#include <NTPClient.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <ErriezDS1307.h>
#include <elapsedMillis.h>
#include "ESPAsyncWebServer.h"

#define DS1307_CTRL_ID 0x68
#define INT_PIN 4
#define ledPin 2

//Network credentials
const char* ssid = "Stofa24867"; 
const char* password =  "stile13fancy15";
int networkList = 0;
bool parse = false;
bool config = false;

//Stepper motor A
const int STEP_A = 23; // Driver p28 for f(scx)A
const int DIR_A = 19;  // Driver p27 for CW/CCW
//Stepper motor D
const int STEP_D = 18; // Driver p17 for f(scx)A
const int DIR_D = 5;   // Driver p16 for CW/CCW

const int RESET = 15;  // pin for RESET
const int TOTALSTEPS = 360 * 12; // of 360 available

// Stepper Travel Variables
int motorsFinished = 0;          // Used to break motors while loop
int move_finished_hour = 1;     // Used to check if hour pointer is completed
int move_finished_min = 1;      // Used to check if minute pointer is completed
long initial_homing = -1;       // Used to Home Stepper at startup

//Create motor objects
AccelStepper motor_hours(AccelStepper::DRIVER, STEP_A, DIR_A);
AccelStepper motor_minutes(AccelStepper::DRIVER, STEP_D, DIR_D);

// Create DS1307 RTC object
ErriezDS1307 rtcSQW;

//NTP server request time data
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String weekDays[7]={};
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

AsyncWebServer server(80);

//RTC module
tmElements_t tm;

// Setup ISR handler
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
long ConvHourToRotation(int hours);
long ConvMinToRotation(int minutes);
void print2digits(int number); 
bool SetTime(int, int, int);
bool SetDate(const char *str);
void ScanForWiFiSignal();
void ConnectToWiFi(const char *, const char *);
void WiFiConnectStatus(const char *);
void Homing();

void setup() {

  // Initialize serial port
  delay(500);
  Serial.begin(115200);

  // Initialize pins
  pinMode(ledPin, OUTPUT);
  pinMode(RESET, OUTPUT);

  digitalWrite(RESET, LOW);
  delay(1);  // keep reset low min 1ms
  digitalWrite(RESET, HIGH);

  // Attach to INT0 interrupt falling edge
  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), sqwHandler, FALLING);

  //Connect to wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  //Setup NTP time
  timeClient.begin(); 
  timeClient.setTimeOffset(7200);
  timeClient.update();

  // Initialize TWI
  //Wire.begin();
  //Wire.setClock(400000);

  // Initialize RTC
  while (!rtcSQW.begin()) {
      Serial.println(F("RTC not found"));
      delay(3000);
  }

  // Enable RTC clock
  if (!rtcSQW.isRunning()) {
      rtcSQW.clockEnable();
  }

  // Set 1Hz square wave out:
  rtcSQW.setSquareWave(SquareWave1Hz);

  //Set the time and date in RTC
  if (SetDate(__DATE__) && SetTime(timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds())) 
  {
    parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm)) 
    {
      config = true;
    }
  }
  //homing the hours and minutes pointers
  delay(5000);
  Homing();
}

void loop() {
  delay(2000);

  //Reads and displays time from RTC tm
  if (RTC.read(tm) && parse && config) 
  {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
  }
}

//  ----------------------- Connecting to the internet -----------------------
void ScanForWiFiSignal()
{
  // WiFi.scanNetworks will return the number of networks found
  networkList = WiFi.scanNetworks();
  Serial.println("scan done");
  if (networkList == 0) 
  {
    Serial.println("no networks found, retrying in 5 seconds");
  } 
  else 
  {
    Serial.print(networkList);
    Serial.println(" networks found");

    for (int i = 0; i < networkList; ++i) 
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
    // Wait a bit before scanning again
    delay(5000);
  }
}

void ConnectToWiFi(const char *DesiredSSID, const char *PassPhrase)
{
  //check if desired SSID is in the list of scanned networks
  for (int i = 0; i < networkList; i++)
  {
    if(WiFi.SSID(i) == DesiredSSID)
    {
      WiFi.begin(DesiredSSID, PassPhrase);
    }
    if(WiFi.SSID(i) == "NETGEAR19")
    {
      WiFi.begin("NETGEAR19", "flamefire2.4");
      WiFiConnectStatus("NETGEAR19");
    }
    if(WiFi.SSID(i) == "NETGEAR19-5G")
    {
      WiFi.begin("NETGEAR19-5G", "flamefire5");
      WiFiConnectStatus("NETGEAR19-5G");
    }
    if(WiFi.SSID(i) == "Stofa24867")
    {
      WiFi.begin("Stofa24867", "stile13fancy15");
      WiFiConnectStatus("Stofa24867");
    }
  }
}

void WiFiConnectStatus(const char* ssid)
{
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" ..");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(1000);
  }
  
  Serial.println();
  Serial.print(ssid);
  Serial.print(" with IP: ");
  Serial.print(WiFi.localIP());
  Serial.print(" -- OK");
}


//   ----------------------- Setting the RTC time -----------------------
bool SetTime(int hour, int min, int sec)
{
  tm.Hour = hour;
  tm.Minute = min;
  tm.Second = sec;
  return true;
}

bool SetDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

void print2digits(int number) 
{
  if (number >= 0 && number < 10) 
  {
    Serial.write('0');
  }
  Serial.print(number);
}


//   ----------------------- Convert time into 360 degrees format  -----------------------
long ConvHourToRotation(int hours)
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
long ConvMinToRotation(int minutes)
{
  return minutes*6; // 1 minute equals 6 degrees
}



//   ----------------------- Run motors until destination is reached  -----------------------
void MoveSteppersInDeg(long minuteMotorPos, long hourMotorPos)
{
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

  motorsFinished = 0;
  while (motorsFinished < 2) // if both motors are done, then break while loop
  {
    motor_minutes.moveTo(minuteMotorPos*12);  // Set moveTo pos of minute motor
    motor_hours.moveTo(hourMotorPos*12);  // Set moveTo pos of hour motor

    // check that pos is within 360 deg
    if ((minuteMotorPos >= 0 && minuteMotorPos <= 360) && (hourMotorPos >= 0 && hourMotorPos <= 360))  
    {

      // if minute and hour motor has not reached desired position -> step
      if (motor_minutes.distanceToGo() != 0)  
      {
        motor_minutes.run(); 

        if (motor_minutes.distanceToGo() == 0) 
        {
          Serial.print("Minute motor(");
          Serial.print(motor_minutes.speed());
          Serial.print(", ");
          Serial.print(motor_minutes.currentPosition());
          Serial.print(") - ");
          Serial.print("COMPLETED!");
          Serial.println("");
          motorsFinished = motorsFinished + 1;  // increment to break while loop
        } 
      }
      if (motor_hours.distanceToGo() != 0)  
      {
        motor_hours.run();

        if (motor_hours.distanceToGo() == 0) 
        {
          Serial.print("Hour motor(");
          Serial.print(motor_hours.speed());
          Serial.print(", ");
          Serial.print(motor_hours.currentPosition());
          Serial.print(") - ");
          Serial.print("COMPLETED!");
          Serial.println("");
          motorsFinished = motorsFinished + 1; // increment to break while loop
        }
      }
    }
  }
}

void Homing()
{
  RTC.read(tm); //read time from RTC

  motor_minutes.setPinsInverted(false);
  motor_hours.setPinsInverted(true);

  //Start homing procedure of stepper motor at startup
  Serial.print("Stepper is Homing to (");  
  Serial.print(tm.Hour);
  Serial.print(" : ");
  Serial.print(tm.Minute);  
  Serial.println(") . . . . . . . . . . .  \n"); 

  motor_minutes.setCurrentPosition(180*12);   // sets the current position as 180 degress in steps for now
  motor_minutes.setMaxSpeed(1000);            // Set Max Speed of Stepper (Slower to get better accuracy)
  motor_minutes.setAcceleration(1000);        // Set Acceleration of Stepper

  motor_hours.setCurrentPosition(180*12);   // sets the current position as 180 degress in steps for now
  motor_hours.setMaxSpeed(1000);            // Set Max Speed of Stepper (Slower to get better accuracy)
  motor_hours.setAcceleration(1000);        // Set Acceleration of Stepper

  initial_homing = 1;
  
  // One hour = 30 degrees
  // 30 min = 50 % of 1 hour = 15 degrees 
  MoveSteppersInDeg(ConvMinToRotation(tm.Minute), ConvHourToRotation(tm.Hour) + (tm.Minute/2)); //we need to move the hour pointer i tad more, all depending on the minutes

  Serial.println("Homing Completed");
  Serial.println("");
}
