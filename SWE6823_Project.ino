//  Author:   John Blake
//  Scheduled Pet Feeder Project

//  Instructor: Michael Franklin
//  Kennesaw State University
//  Spring Semester 2025
//  SWE 6823 Embedded Systems

#include <I2C_RTC.h> // For the RTC module
#include <Arduino.h> // Arduino code environment
#include <Wire.h> // For I2C communication
#include "SystemUI.h"
#include "TimeMgmt.h"
#include "TimeValue.h"
#include "Debug.h"
#include "SystemUtil.h"
#include "DoorMgmt.h"

#pragma region Global_Variables
// Pin number (Constant)
const uint8_t 
  LED_Dispensing = 7,
  Btn_Up = 2, Btn_Down = 4, Btn_OK = 5, Btn_Menu = 6,
  Direction = 12, PWM = 3, Brake = 9,
  Photoresistor = A1;
// True = the current input is already handled, otherwise False
bool isInputHandled, isDispensing;
// True = button is pressed, otherwise False
bool upPressed, downPressed, okPressed, menuPressed;
// The amount of time (ms) between running this task's code.
const long intervalTime = 1000, intervalUi = 10, intervalDoorCheck = 100;
volatile bool isTimeReady = false, isUiReady = false, isSystemResetReady = false, isDoorCheckReady = false;

bool runWithDebug = false; // Enable debugging/diagnostic printing

// TODO: error led?
#pragma endregion Global_Variables

// Runs once upon startup.
void setup() {
  // Pin modes
  pinMode(LED_Dispensing, OUTPUT);
  pinMode(Btn_Up, INPUT);
  pinMode(Btn_Down, INPUT);
  pinMode(Btn_OK, INPUT);
  pinMode(Btn_Menu, INPUT);
  pinMode(Photoresistor, INPUT);
  // Initialize processes.
  DoorMgmt::Init(Direction, PWM, Brake, Photoresistor);
  SystemUI::Init(runWithDebug, "v1.0.1");
  TimeMgmt::Init();
  
  // Debugger uses Serial Monitor
  if (runWithDebug) {
    Debug::Init();
    // Other debugging code
    for (int i = 0 ; i < 12; i++) { // Add full schedule of times
      TimeMgmt::setScheduleTime(i, 0, i*2 + 1, i*3);
    } 
  }

  // Force close the door.
  DoorMgmt::forceCloseDoor(true);
}

#pragma region Helper_Methods
// Updates the ___Pressed variables: True = pressed, False = not pressed.
void readInput() {
  upPressed = digitalRead(Btn_Up) == HIGH;
  downPressed = digitalRead(Btn_Down) == HIGH;
  okPressed = digitalRead(Btn_OK) == HIGH;
  menuPressed = digitalRead(Btn_Menu) == HIGH;
}
// Updates the LED and `isDispensing` variable based on value of arg.
void toggleDispensingStatus(bool arg) {
  if (arg) {
    isDispensing = true;
    digitalWrite(LED_Dispensing, HIGH);
    Debug::println("It is now feeding time!");
  }
  else {
    isDispensing = false;
    digitalWrite(LED_Dispensing, LOW);
    Debug::println("Done with feeding routine.");
  }
}

#pragma endregion Helper_Methods

// "Main" code, loops indefinitely.
void loop() {
  // The # of ms elapsed since the program started.
  unsigned long now = millis();

  isTimeReady = now % intervalTime == 0;
  isUiReady = now % intervalUi == 0;
  isDoorCheckReady = now % intervalDoorCheck == 0;
  isSystemResetReady = SystemUI::IsResetReady();

  // [Time Task]: Update time, check for schedule time (once per 1 s)
  if (isTimeReady) {
    // Check if current time is feeding time
    bool isFoodTime = TimeMgmt::isFeedingTime();

    // If this time is a time on the food schedule and we are not already dispensing,
    if (isFoodTime && !isDispensing) {
      // Then start dispensing routine.
      toggleDispensingStatus(true);
      SystemUI::SetText(String("Dispensing food."), -1);
      DoorMgmt::dispenseFood();
    }
    // If doormgmt is done with dispensing routine,
    if (DoorMgmt::isDispensingFood() == false && isDispensing == true) {
      // Then stop routine and resume UI.
      toggleDispensingStatus(false);
      SystemUI::UnpauseUi();
      SystemUI::UpdateUI();
    }
    
    // sync SystemUI's currentTime with the current time from timeMgmt
    SystemUI::UpdateTime(TimeMgmt::getSysTime());
    if (SystemUI::ErrorTick()) { // clock tick for error message on a time limit 
      SystemUI::UnpauseUi();
      SystemUI::UpdateUI();
    }

    //(Debug only)
    if (runWithDebug) {
      Debug::println(String(DoorMgmt::detectJam()) + " = Door jam");
    }
  }
  // [UI Task]: Read inputs, update UI (once per 10 ms)
  if (isUiReady) {
    // Check for input
    readInput();
    if (upPressed || downPressed || okPressed || menuPressed) {
      // If just one button is pressed,
      if (!isInputHandled && (upPressed + downPressed + okPressed + menuPressed == 1)) {
        // Respond to button input
        if (upPressed) {
          Debug::println("UP pressed");
          SystemUI::Input(UiButton::Up);
        }
        if (downPressed) {
          Debug::println("DOWN pressed");
          SystemUI::Input(UiButton::Down);
        }
        if (okPressed) {
          Debug::println("OK pressed");
          SystemUI::Input(UiButton::OK);
          if (DoorMgmt::isDispensingFood()) {
            DoorMgmt::okPressedHandler();
          }
        }
        if (menuPressed) {
          Debug::println("MENU pressed");
          SystemUI::Input(UiButton::Menu);
        }
        SystemUI::UpdateUI();
      }

      isInputHandled = true;
    }
    else {
      isInputHandled = false;
    }
    // Update UI as needed.
    if (SystemUI::IsTimeNeeded() && isTimeReady) {
      SystemUI::UpdateUI();
    }
  }

  // [Door Task]: For operating door (clock tick once per 0.1 s)
  if (isDoorCheckReady) {
    DoorMgmt::ClockTick();
  }

  // This is not a task like the others. If the user enters OK when prompted to reset system, this occurs.
  if (isSystemResetReady) {
    SystemUI::SetText("Resetting.", 5);
    delay(1000);
    systemReset(true); // in SystemUtil.cpp
  }
}
