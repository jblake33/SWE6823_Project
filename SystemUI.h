#ifndef SYSTEMUI_H
#define SYSTEMUI_H

#include <Arduino.h> // Arduino code environment
#include "TimeValue.h"

enum class UiButton {
  Up = 0,
  Down = 1,
  OK = 2,
  Menu = 3
};

// The state the UI is in.
enum class UiState {
  Home = 0,
  Menu = 1,
  ScheduleMenu = 2,
  ViewTimes = 3,
  SetTimes = 4,
  RemoveTimes = 5,
  SystemMenu = 6,
  SetTime = 7,
  SystemInfo = 8,
  Reset = 9,
  TimeInput = 10
};

class SystemUI {
  public:
    static void Init(bool isDebugEnabled = false, String verNum = "unknown");
    // Disables UI input and sets on-screen text to msg.
    // Use \n for multiple lines.
    // Accepts optional arg for amount of time, default 5 seconds.
    static void SetText(String msg, uint8_t timeDelay);
    // Takes input for the new system time value to display
    static void UpdateTime(TimeValue* newTime);
    // Input handler for the buttons.
    static void Input(UiButton i);
    // Updates the text displayed on the LCD display.
    static void UpdateUI();
    // Uses the currentState to decide if the menu should be printed again, with the new time.
    static bool IsTimeNeeded();
    // Returns true if user has inputted ready for system reset.
    static bool IsResetReady();
    // Getter for whether the UI is paused.
    static bool IsUiPaused();
    // Setter for pausing the UI.
    static void PauseUi();
    // Setter for unpausing the UI.
    static void UnpauseUi();
    // Used by the driver to "tick" error delay until it runs out.
    // Returns true if error delay is done and ready to update ui
    static bool ErrorTick();
    // Clears error message from the screen, unpauses and updates UI,
    static void ClearError();

  private:
    // The logic for how each button input is handled on each UI screen.
    static void HomeUi(UiButton i);
    static void MenuUi(UiButton i);
    static void ScheduleMenuUi(UiButton i);
    static void SysMenuUi(UiButton i);
    static void ViewTimesUi(UiButton i);
    static void SetTimesUi(UiButton i);
    static void RemoveTimesUi(UiButton i);
    static void SetSysTimeUi(UiButton i);
    static void SysInfoUi(UiButton i);
    static void ResetUi(UiButton i);
    static void TimeInputUi(UiButton i);

    // The metehods called to print UI text to the screen.
    static void PrintHomeUi();
    static void PrintMenuUi();
    static void PrintScheduleMenuUi();
    static void PrintSysMenuUi();
    static void PrintViewTimesUi();
    static void PrintSetTimesUi();
    static void PrintRemoveTimesUi();
    static void PrintSetSysTimeUi();
    static void PrintSysInfoUi();
    static void PrintResetUi();
    static void PrintTimeInputUi();
};
#endif