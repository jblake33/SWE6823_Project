#include "TimeMgmt.h"
#include "TimeValue.h"
#include "Schedule.h" // 
#include <Arduino.h> // Arduino code environment
#include <I2C_RTC.h> // For the RTC module
#include <Wire.h> // For I2C communication
#include "Debug.h"

// State variables
static DS3231 RTC;
Schedule* TimeMgmt::foodSchedule = nullptr;

static void TimeMgmt::Init() {
  RTC.begin();
  RTC.setYear(2000);
  RTC.setMonth(1);
  RTC.setDay(1);
  RTC.setHours(0);
  RTC.setMinutes(0);
  RTC.setSeconds(0);
  foodSchedule = new Schedule();
}

static uint8_t TimeMgmt::getSeconds() {
  return RTC.getSeconds();
}
static uint8_t TimeMgmt::getMinutes() {
  return RTC.getMinutes();
}
static uint8_t TimeMgmt::getHours() {
  return RTC.getHours();
}

static TimeValue* TimeMgmt::getSysTime() {
  TimeValue* t = new TimeValue();
  t->seconds = getSeconds();
  t->minutes = getMinutes();
  t->hours = getHours();
  return t;
}

static bool TimeMgmt::setSeconds(uint8_t s) {
  if (s < 60) {
    RTC.setSeconds(s);
    return true;
  }
  return false;
}
static bool TimeMgmt::setMinutes(uint8_t m) {
  if (m < 60) {
    RTC.setMinutes(m);
    return true;
  }
  return false;
}
static bool TimeMgmt::setHours(uint8_t h) {
  if (h < 24) {
    RTC.setHours(h);
    return true;
  }
  return false;
}
static uint8_t TimeMgmt::getScheduleSize() {
  return TimeMgmt::foodSchedule->getCount();
}
static TimeValue TimeMgmt::getScheduleTime(uint8_t index) {
  return TimeMgmt::foodSchedule->getTime(index);
}

// Returns: 1 = success, 2 = failed (time conlflict), 0 = failed (bad index)
static uint8_t TimeMgmt::setScheduleTime(uint8_t index, uint8_t h, uint8_t m, uint8_t s) {
  if (index == TimeMgmt::foodSchedule->getCount()) {
    // Add time
    return TimeMgmt::foodSchedule->addTime(h, m, s);
  }
  else {
    // Update time
    return TimeMgmt::foodSchedule->updateTime(index, h, m, s);
  }
}
static bool TimeMgmt::removeScheduleTime(uint8_t index) {
  return TimeMgmt::foodSchedule->removeTime(index);
}

// Returns true if the current time is a feeding time in the schedule.
static bool TimeMgmt::isFeedingTime() {
  uint8_t s = TimeMgmt::foodSchedule->getCount();
  if (s == 0) return false;
  TimeValue* ct = TimeMgmt::getSysTime();
  long time_seconds = ct->totalSeconds(), tmp_seconds;
  delete ct;
  for (int i = 0; i < s; i++) {
    tmp_seconds = TimeMgmt::getScheduleTime(i).totalSeconds();
    if (time_seconds == tmp_seconds) {
      // This is feeding time
      return true;
    }
    if (time_seconds < tmp_seconds) {
      // since the schedule is smallest time first, and our search is linear, if our current time
      // is before the feeding time we are looking at, it is not feeding time.
      return false;
    }
  }
  return false;
}