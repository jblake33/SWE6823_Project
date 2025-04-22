#include "DoorMgmt.h"
#include <Arduino.h>
#include "Debug.h"
#include "SystemUI.h"

// Pin value on board
static uint8_t direction, pwm, brake, photoresistor;

// The motor's work duty
const static int workDuty = 250;
// Multiply this value by intervalDoorCheck in main to get 
// # ms time a door moves in a direction start to finish.
const static int doorTime_ms = 25;

const static String jamErrorMsg = String("[Error]\nJam detected.\nRemove jam, then\npress OK to resume.");
// State variables
static bool doorIsOpen, doorIsMoving, doorDirectionIsOpen, isDoingFoodDispensal, isOkPressed, isDoorJammed;
static int doorMovingDuration;

// Sets door moving duration to ms. Also sets doorIsMoving = true
static void DoorMgmt::setDoorDuration(int ms) { 
  doorMovingDuration = ms;
  doorIsMoving = true;
}
// Sets door direction and updates direction pin accordingly.
static void DoorMgmt::setDoorDirection(bool isOpenDirection) {
  doorDirectionIsOpen = isOpenDirection;
  // Sets direction pin. HIGH/LOW could be swapped to reverse the physical direction.
  digitalWrite(direction, (doorDirectionIsOpen ? HIGH : LOW));
}

// Returns true = door is open, false = door is closed
static bool DoorMgmt::isDoorOpen() {
  return doorIsOpen;
}
// Returns true = door is moving, false = door is closed
static bool DoorMgmt::isDoorMoving() {
  return doorIsMoving;
}

// Returns true = currently dispensing food, false = idle
static bool DoorMgmt::isDispensingFood() {
  if (doorMovingDuration > 0 && !isDoingFoodDispensal) {
    Debug::println("WARN: Dispensal false, but duration = " + String(doorMovingDuration));
  }
  return isDoingFoodDispensal;
}

// Force stops the door.
static void DoorMgmt::forceStopDoor() {
  Debug::println("Stopping door.");
  // Set motor's work load to 0
  analogWrite(pwm, 0);
  // Enable brakes.
  digitalWrite(brake, HIGH);
  // Update doorMoving.
  doorIsMoving = false;
}

// Force opens the door, after checking if the door is closed and not moving. 
// Override skips the check.
static void DoorMgmt::forceOpenDoor(bool override = false) {
  if (override) {
    Debug::println("Opening door.");
    DoorMgmt::setDoorDirection(true);
    analogWrite(pwm, workDuty);
    digitalWrite(brake, LOW);
    delay(1000);
    DoorMgmt::forceStopDoor();
    return;
  }

  // if the door is closed and not moving,
  if (!DoorMgmt::isDoorOpen() && !DoorMgmt::isDoorMoving()) {
    // then start the "open door" routine
    Debug::println("Opening door.");
    DoorMgmt::setDoorDirection(true);
    DoorMgmt::setDoorDuration(doorTime_ms);
    analogWrite(pwm, workDuty);
    digitalWrite(brake, LOW);
  }
}

// Force closes the door, after checking if the door is open and not moving. 
// Override skips the check.
static void DoorMgmt::forceCloseDoor(bool override = false) {
  // Override ignores whether it is feeding time or the door is open or not.
  if (override) {
    Debug::println("Closing door.");
    DoorMgmt::setDoorDirection(false);
    analogWrite(pwm, workDuty);
    digitalWrite(brake, LOW);
    delay(1000);
    DoorMgmt::forceStopDoor();
    return;
  }
  
  // if the door is open and not moving,
  if (DoorMgmt::isDoorOpen() && !DoorMgmt::isDoorMoving()) {
    Debug::println("Closing door.");
    // then start the "close door" routine
    DoorMgmt::setDoorDirection(false);
    DoorMgmt::setDoorDuration(doorTime_ms);
    analogWrite(pwm, workDuty);
    digitalWrite(brake, LOW);
  }
}

// To be called in system monitoring to update door process
static void DoorMgmt::ClockTick() {
  if (isDoorJammed) {
    // When the user presses OK,
    if (isOkPressed) {
      Debug::println("Attempting door again");
      isOkPressed = false;
      SystemUI::SetText("Resuming...", -1);
      if (doorDirectionIsOpen) {
        DoorMgmt::forceOpenDoor(true);
      }
      else {
        DoorMgmt::forceCloseDoor(true);
      }
      // detect jam
      if (DoorMgmt::detectJam() == false) {
        // if no jam, update ui text (the isDoorJammed is updated in detectJam)
        SystemUI::SetText("Dispensing food.", 2);
        goto resume_food; // Jam error is resolved; go to resume food.
      }
      else {
        SystemUI::SetText(jamErrorMsg, -1);
      }
    }
    return;
  }
  if (doorIsMoving) {
    doorMovingDuration--;
    Debug::println(String(doorMovingDuration) + " dur");
  }
  // if door is done moving,
  if (doorMovingDuration <= 0 && doorIsMoving) {
    doorIsOpen = doorDirectionIsOpen; // doorIsOpen is updated
    // If there is a jam,
    if (DoorMgmt::detectJam()) {
      // Alert to the user. Message is cleared when jam is resolved.
      SystemUI::SetText(jamErrorMsg, -1);
      DoorMgmt::forceStopDoor();
      return;
    }
    resume_food:
    // If we are on the food routine, (there is no jam)
    if (isDoingFoodDispensal) {
      // and the door is open,
      if (doorIsOpen) {
        // then stop and close the door
        DoorMgmt::forceStopDoor();
        delay(1000);
        DoorMgmt::forceCloseDoor();
      }
      else {
        // This means door is closed and we are done with food routine.
        DoorMgmt::forceStopDoor();
        isDoingFoodDispensal = false;
      }
    }
  }
}

static void DoorMgmt::Init(uint8_t directionPin, uint8_t pwmPin, uint8_t brakePin, uint8_t photoresistorPin) {
  // Setting pin values
  direction = directionPin;
  pwm = pwmPin;
  brake = brakePin;
  photoresistor = photoresistorPin;

  // Setting other status variables
  doorMovingDuration = 0;
  doorIsOpen = false;
  doorIsMoving = false;
  isDoingFoodDispensal = false;
  isDoorJammed = false;
  isOkPressed = false;
  DoorMgmt::setDoorDirection(false); // default in the closing direction
}

// Starts the food dispensal routine.
// To be called from system management.
static void DoorMgmt::dispenseFood() {
  isDoingFoodDispensal = true;
  DoorMgmt::forceOpenDoor(); // Start food dispensal by opening door
  // ClockTick will forceCloseDoor at appropriate time.
}

// Returns true if jam detected, returns false otherwise.
// Updates value of `isDoorJammed` accordingly.
static bool DoorMgmt::detectJam() {
  // high value read = light = door open
  // low value read = no light = door closed
  // If door closes and light is still high, there is a jam.
  // If door opens and light is still low, there is a jam.

  uint8_t high_reading = 40; // Adjust values as needed.
  uint8_t low_reading = 20;

  int read = analogRead(photoresistor);
  // If door should be open, but sensor reads door closed (or vice versa),
  if (doorIsOpen && read < low_reading) {
    Debug::println("Door: " + String(read) + " < " + String(low_reading));
    isDoorJammed = true;
    return true; // Jam
  }
  
  if (!doorIsOpen && read > high_reading) {
    Debug::println("Door: " + String(read) + " > " + String(high_reading));
    isDoorJammed = true;
    return true; // Jam
  }

  Debug::println("Door: " + String(read));
  isDoorJammed = false;
  return false; // No jam (expected)
}

static void DoorMgmt::okPressedHandler() {
  isOkPressed = true;
  Debug::println("Okpressed = " + String(isOkPressed));
}