#include <Arduino.h>
#include <DS1307RTC.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ErriezDS1307.h>

//Includes handling of NTP time and RTC time
class Time
{
private:
    const char *monthName_[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    String weekDays[7]={};
    ErriezDS1307 rtcSQW_;
public:
    Time(DS1307RTC, tmElements_t, NTPClient);
    ~Time();
    void SetupNTP();
    void SetupRTCInterrupt();
    bool SetTime(int hour, int min, int sec);
    bool SetDate(const char *str);
    void SetTimeInRTC();
    void GetTimeFromNTPServer();

    DS1307RTC RTC_;
    tmElements_t tm_;
    NTPClient timeClient_;
    bool RTCConfig_;
    bool timeParse_;
};
