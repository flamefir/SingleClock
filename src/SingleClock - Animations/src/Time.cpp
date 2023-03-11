#include <Time.hpp>

Time::Time(DS1307RTC RTC, tmElements_t tm, NTPClient timeClient) : RTC_(RTC), tm_(tm), timeClient_(timeClient)
{
}

Time::~Time()
{
}

void Time::SetupNTP()
{
  timeOffset_ = (DST_)? 7200 : 3600; 
  timeClient_.begin(); 
  timeClient_.setTimeOffset(timeOffset_); //7200 summer time // 3600 winter time
  timeClient_.update();
}

void Time::SetupRTCInterrupt()
{
    // Initialize RTC
    while (!rtcSQW_.begin()) 
    {
        Serial.println(F("RTC not found"));
        delay(3000);
    }

    // Enable RTC clock
    if (!rtcSQW_.isRunning()) 
    {
        rtcSQW_.clockEnable();
    }

    // Set 1Hz square wave out:
    rtcSQW_.setSquareWave(SquareWave1Hz);
}

bool Time::SetTime(int hour, int min, int sec)
{
  tm_.Hour = hour;
  tm_.Minute = min;
  tm_.Second = sec;
  return true;
}

bool Time::SetDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName_[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm_.Day = Day;
  tm_.Month = monthIndex + 1;
  tm_.Year = CalendarYrToTm(Year);
  return true;
}

 void Time::UpdateDST(tmElements_t tm_)
 {
  /*
    EU standard for DST daylight saving time is: 
    - One hour backwards the last sunday in October
    - One hour forwards the last sunday in March
  */
  if(tm_.Month == 10 && tm_.Day >= 25 && tm_.Wday == 1 && tm_.Hour == 3)
  {
    SetTime(timeClient_.getHours() - 1, timeClient_.getMinutes(), timeClient_.getSeconds());
    DST_ = false;
    timeParse_ = true;
    if (RTC_.write(tm_)) 
    {
      RTCConfig_ = true;
    }
    else {RTCConfig_ = false;}
  }
  
  if(tm_.Month == 3 && tm_.Day >= 25 && tm_.Wday == 1 && tm_.Hour == 2)
  {
    SetTime(timeClient_.getHours() + 1, timeClient_.getMinutes(), timeClient_.getSeconds());
    DST_ = true;
    timeParse_ = true;
    if (RTC_.write(tm_)) 
    {
      RTCConfig_ = true;
    }
    else {RTCConfig_ = false;}
  }
 }

void Time::SetTimeInRTC()
{
  if (SetDate(__DATE__) && SetTime(timeClient_.getHours(), timeClient_.getMinutes(), timeClient_.getSeconds())) 
  {
    timeParse_ = true;
    // and configure the RTC with this info
    if (RTC_.write(tm_)) 
    {
      RTCConfig_ = true;
    }
  }
}