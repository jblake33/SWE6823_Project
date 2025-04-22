#ifndef TIMEVALUE_H
#define TIMEVALUE_H

#include <Arduino.h> // Arduino code environment

class TimeValue {
  public:
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    bool isValid();
    String toString();
    long totalSeconds();
};

#endif