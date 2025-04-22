#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <Arduino.h> // Arduino code environment
#include "TimeValue.h"

class Schedule {
  protected:
    void sort();
    TimeValue* schedule;
    uint8_t count;
    bool checkTimeConflicts(uint8_t h, uint8_t m, uint8_t s, bool do_skip, uint8_t skip_index);
  public: 
    Schedule();
    TimeValue getTime(uint8_t index);
    uint8_t addTime(uint8_t h, uint8_t m, uint8_t s);
    uint8_t updateTime(uint8_t index, uint8_t h, uint8_t m, uint8_t s);
    bool removeTime(uint8_t index);
    uint8_t getCount();
};

#endif