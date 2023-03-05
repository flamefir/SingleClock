#include <Time.hpp>

Time::Time(DS1307RTC RTC, tmElements_t tm) : RTC_(RTC), tm_(tm)
{
}

Time::~Time()
{
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

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3)
    return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++)
  {
    if (strcmp(Month, monthName_[monthIndex]) == 0)
      break;
  }
  if (monthIndex >= 12)
    return false;
  tm_.Day = Day;
  tm_.Month = monthIndex + 1;
  tm_.Year = CalendarYrToTm(Year);
  return true;
}

// TODO get input from bluetooth serial and use here
void Time::SetTimeInRTC()
{
  if (SetDate(__DATE__) && SetTime(tm_.Hour, tm_.Minute, tm_.Second))
  {
    timeParse_ = true;
    // and configure the RTC with this info
    if (RTC_.write(tm_))
    {
      RTCConfig_ = true;
    }
  }
}