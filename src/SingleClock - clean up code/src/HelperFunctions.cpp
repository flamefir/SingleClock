#include <HelperFunctions.hpp>


HelperFunction::HelperFunction(DS1307RTC RTC, tmElements_t tm) : RTC_(RTC), tm_(tm)
{
}

HelperFunction::~HelperFunction()
{
}

int HelperFunction::GetDigitInt(int num, int index)
{
  int r = num / pow(10, index);
  return r % 10;
}

void HelperFunction::Print2digits(int number) 
{
  if (number >= 0 && number < 10) 
  {
    Serial.write('0');
  }
  Serial.print(number);
}

void HelperFunction::PrintTimeFromRTC(bool parse_, bool config_)
{
  //Reads and displays time from RTC tm
  if (RTC_.read(tm_) && parse_ && config_) 
  {
    Serial.print("Ok, Time = ");
    Print2digits(tm_.Hour);
    Serial.write(':');
    Print2digits(tm_.Minute);
    Serial.write(':');
    Print2digits(tm_.Second);
    Serial.println("");
  }
}

void HelperFunction::PrintDateFromRTC(bool parse_, bool config_)
{
  //Reads and displays time from RTC tm
  if (RTC_.read(tm_) && parse_ && config_) 
  {
    Serial.print("Date (D/M/Y) = ");
    Serial.print(tm_.Day);
    Serial.write('/');
    Serial.print(tm_.Month);
    Serial.write('/');
    Serial.println(tmYearToCalendar(tm_.Year));
  }
}