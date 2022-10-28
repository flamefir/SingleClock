#pragma once
#include <DS1307RTC.h>
#include <Arduino.h>

class HelperFunction
{
private:
    DS1307RTC RTC_;
    tmElements_t tm_;
public:
    HelperFunction(DS1307RTC, tmElements_t);
    ~HelperFunction();
    int GetDigitInt(int, int);
    void Print2digits(int);
    void PrintTimeFromRTC(bool parse_, bool config_);
};
