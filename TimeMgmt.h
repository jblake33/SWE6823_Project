#ifndef TIMEMGMT_H
#define TIMEMGMT_H

#include <Arduino.h> // Arduino code environment
#include "TimeValue.h"
#include "Schedule.h"

class TimeMgmt {
  private:
    
  public:
    static Schedule* foodSchedule;
    static void Init();
    static uint8_t getSeconds();
    static uint8_t getMinutes();
    static uint8_t getHours();
    static TimeValue* getSysTime();
    static bool setSeconds(uint8_t s);
    static bool setMinutes(uint8_t m);
    static bool setHours(uint8_t h);
    static uint8_t getScheduleSize();
    static TimeValue getScheduleTime(uint8_t index);
    static uint8_t setScheduleTime(uint8_t index, uint8_t h, uint8_t m, uint8_t s);
    static bool removeScheduleTime(uint8_t index);
    static bool isFeedingTime();
};

#endif