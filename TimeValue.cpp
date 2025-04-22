#include "TimeValue.h"
#include <Arduino.h> // Arduino code environment

bool TimeValue::isValid() {
  return seconds < 60 && minutes < 60 && hours < 24;
}

String TimeValue::toString() {
  return (hours < 10 ? "0" : "") + String(hours) + ":"
    + (minutes < 10 ? "0" : "") + String(minutes) + ":"
    + (seconds < 10 ? "0" : "") + String(seconds);
}

long TimeValue::totalSeconds() {
  long h = hours;
  long m = minutes;
  long s = seconds;
  return h * 3600 + m * 60 + s;
}