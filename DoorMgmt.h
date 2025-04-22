#ifndef DOORMGMT_H
#define DOORMGMT_H

#include "Arduino.h"
class DoorMgmt {
  private:
    static void setDoorDuration(int ms);
    static void setDoorDirection(bool isOperDirection);
  public:
    static void Init(uint8_t directionPin, uint8_t pwmPin, uint8_t brakePin, uint8_t photoresistorPin);
    static void forceOpenDoor(bool override = false);
    static void forceCloseDoor(bool override = false);
    static void forceStopDoor();
    static void dispenseFood();
    static bool detectJam();
    static bool isDoorOpen();
    static bool isDoorMoving();
    static bool isDispensingFood();
    static void okPressedHandler();
    // To be called once per ~100 ms
    static void ClockTick();
};

#endif