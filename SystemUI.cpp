#include "SystemUI.h"
#include <Arduino.h> // Arduino code environment
#include <LiquidCrystal_I2C.h> // for the LCD
#include "TimeValue.h"
#include "TimeMgmt.h"
#include "Debug.h" // For debugging (could be removed)

#pragma region State_Vars
LiquidCrystal_I2C lcd(0x27, 20, 4);
UiState currentState;
byte mainMenuCursorPos, scheduleMenuCursorPos, systemMenuCursorPos;
byte timeSelectCursorPos, timeAdjustCursorPos;
bool debugEnabled;
String version;
TimeValue* currentTime;
TimeValue* tmp_time;
bool readyForReset, isPaused;
uint8_t errorDelay;
enum TimeInputFallback {
  SetScheduleTimes = 0,
  SetSysTime = 1
};
TimeInputFallback formPrevUi = 0;
String timeInputHeader;
// Up arrow custom character (8 row, 5 col pixels)
byte upArrow[] = {
  B00000,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00000
};
// Down arrow custom character (8 row, 5 col pixels)
byte downArrow[] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
  B00000
};
#pragma endregion State_Vars

static void SystemUI::Init(bool isDebugEnabled = false, String verNum) {
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");
  currentState = UiState::Home;
  mainMenuCursorPos = 0;
  scheduleMenuCursorPos = 0;
  systemMenuCursorPos = 0;
  timeSelectCursorPos = 0;
  errorDelay = 0;
  readyForReset = false;
  isPaused = false;
  tmp_time = new TimeValue();
  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);
  version = verNum;
  debugEnabled = isDebugEnabled;
}

#pragma region Helper_Methods
// Disables UI input and sets on-screen text to msg.
// Use \n for multiple lines.
// Accepts optional arg for amount of time, default 5 seconds. 
// `timeDelay < 0` means message does not have a time limit.
static void SystemUI::SetText(String msg, uint8_t timeDelay = 5) {
  Debug::println("Setting UI text to: " + msg);
  errorDelay = timeDelay;
  lcd.clear();
  uint8_t line = 0;
  // Iterate char by char in msg
  for (int i = 0, j = msg.length(); i < j; i++) {
    if (msg[i] == '\n') { // Insert newline if char == \n
      if (line < 4) { // our lcd has 4 rows.
        lcd.setCursor(0, ++line);
      }
      else {
        continue; // Early continue (remaining text won't fit on screen)
      }
    }
    else { // Else print char
      lcd.print(msg[i]);
    }
  }
  // Disable UI Input
  PauseUi();
}

static void SystemUI::UpdateTime(TimeValue* newValue) {
  if (newValue->isValid()) {
    delete currentTime; // Delete deallocates the memory the pointer points to.
    currentTime = newValue;
    Debug::println("Time: " + String(currentTime->toString()));
  }
}

// Returns true if user has inputted ready for system reset.
static bool SystemUI::IsResetReady() {
  return readyForReset;
}
// Getter for whether the UI is paused.
static bool SystemUI::IsUiPaused() {
  return isPaused;
}
// Setter for pausing the UI.
static void SystemUI::PauseUi() {
  isPaused = true;
  Debug::println("Pausing UI");
}
// Setter for unpausing the UI.
static void SystemUI::UnpauseUi() {
  isPaused = false;
  Debug::println("Unpausing UI");
}

static bool SystemUI::ErrorTick() {
  if (errorDelay > 0) {
    errorDelay--;
    if (errorDelay == 0) {
      return true;
    }
  }
  return false;
}

static void SystemUI::ClearError() {
  errorDelay = 0;
  SystemUI::UnpauseUi();
  SystemUI::UpdateUI();
}

static bool SystemUI::IsTimeNeeded() {
  switch (currentState) { 
    // Current time is displayed on these UI screens:
    case UiState::Home:
    case UiState::SetTime:
    case UiState::SystemInfo:
      return true;
  }
  return false;
}
#pragma endregion Helper_Methods

#pragma region Input_Handler_Methods
static void SystemUI::Input(UiButton i) {
  // if the UI is paused, don't take inputs
  if (isPaused) return;
  // Otherwise, pass the inputted button arg to the current Ui's logic method
  switch (currentState) {
    case UiState::Home:
      SystemUI::HomeUi(i);
      break;
    case UiState::Menu:
      SystemUI::MenuUi(i);
      break;
    case UiState::ScheduleMenu:
      SystemUI::ScheduleMenuUi(i);
      break;
    case UiState::SystemMenu:
      SystemUI::SysMenuUi(i);
      break;
    case UiState::ViewTimes:
      SystemUI::ViewTimesUi(i);
      break;
    case UiState::SetTimes:
      SystemUI::SetTimesUi(i);
      break;
    case UiState::RemoveTimes:
      SystemUI::RemoveTimesUi(i);
      break;
    case UiState::SetTime:
      SystemUI::SetSysTimeUi(i);
      break;
    case UiState::SystemInfo:
      SystemUI::SysInfoUi(i);
      break;
    case UiState::Reset:
      SystemUI::ResetUi(i);
      break;
    case UiState::TimeInput:
      SystemUI::TimeInputUi(i);
      break;
  }
}
static void SystemUI::HomeUi(UiButton i) {
  switch (i) {
    case UiButton::Up:
      // Do nothing
      break;
    case UiButton::Down:
      // Do nothing
      break;
    case UiButton::OK:
      // Do nothing
      break;
    case UiButton::Menu:
      currentState = UiState::Menu;
      break;
  }
}
static void SystemUI::MenuUi(UiButton i) {
  switch (i) {
    case UiButton::Up:
      mainMenuCursorPos = (mainMenuCursorPos + 2) % 3;
      break;
    case UiButton::Down:
      mainMenuCursorPos = (mainMenuCursorPos + 1) % 3;
      break;
    case UiButton::OK:
      switch (mainMenuCursorPos) {
        case 0:
          currentState = UiState::ScheduleMenu;
          break;
        case 1:
          currentState = UiState::SystemMenu;
          break;
        case 2:
          currentState = UiState::Home;
          break;
      }
      break;
    case UiButton::Menu:
      currentState = UiState::Home;
      break;
  }
}
static void SystemUI::ScheduleMenuUi(UiButton i) {
  switch (i) {
    case UiButton::Up:
      scheduleMenuCursorPos = (scheduleMenuCursorPos + 3) % 4;
      break;
    case UiButton::Down:
      scheduleMenuCursorPos = (scheduleMenuCursorPos + 1) % 4;
      break;
    case UiButton::OK:
      timeSelectCursorPos = 0; // Reset this position regardless
      switch (scheduleMenuCursorPos) {
        case 0:
          currentState = UiState::ViewTimes;
          break;
        case 1:
          currentState = UiState::SetTimes;
          break;
        case 2:
          currentState = UiState::RemoveTimes;
          break;
        case 3:
          currentState = UiState::Menu;
          break;
      }
      break;
    case UiButton::Menu:
      currentState = UiState::Home;
      break;
  }
}
static void SystemUI::SysMenuUi(UiButton i) {
  switch (i) {
    case UiButton::Up:
      systemMenuCursorPos = (systemMenuCursorPos + 3) % 4;
      break;
    case UiButton::Down:
      systemMenuCursorPos = (systemMenuCursorPos + 1) % 4;
      break;
    case UiButton::OK:
      switch (systemMenuCursorPos) {
        case 0:
          formPrevUi = TimeInputFallback::SetSysTime;
          currentState = UiState::SetTime;
          break;
        case 1:
          currentState = UiState::SystemInfo;
          break;
        case 2:
          currentState = UiState::Reset;
          break;
        case 3:
          currentState = UiState::Menu;
          break;
      }
      break;
    case UiButton::Menu:
      currentState = UiState::Home;
      break;
  }
}
static void SystemUI::ViewTimesUi(UiButton i) {
  uint8_t scheduleSize = TimeMgmt::getScheduleSize();
  switch (i) {
    case UiButton::Up:
      if (timeSelectCursorPos > 0) {
        timeSelectCursorPos --;
      }
      else {
        timeSelectCursorPos = 0;
      }
      break;
    case UiButton::Down:
      timeSelectCursorPos ++;
      if (timeSelectCursorPos > scheduleSize - 1) {
        timeSelectCursorPos = scheduleSize - 1;
      }
      break;
    case UiButton::OK:
      currentState = UiState::ScheduleMenu;
      break;
    case UiButton::Menu:
      currentState = UiState::ScheduleMenu;
      break;
  }
}
static void SystemUI::SetTimesUi(UiButton i) {
  uint8_t scheduleSize = TimeMgmt::getScheduleSize();
  switch (i) {
    case UiButton::Up:
      if (timeSelectCursorPos != 0) {
        timeSelectCursorPos --;
      }
      break;
    case UiButton::Down:
      timeSelectCursorPos ++;
      if (timeSelectCursorPos > scheduleSize) {
        timeSelectCursorPos = scheduleSize;
      }
      if (scheduleSize == 12 && timeSelectCursorPos >= 12) {
        timeSelectCursorPos = 11;
      }
      break;
    case UiButton::OK:
      // The timeSelectCursorPos will be used to determine what times to populate
      if (timeSelectCursorPos == scheduleSize) {
        timeInputHeader = "[Add Time]";
        tmp_time->hours = 0;
        tmp_time->minutes = 0;
        tmp_time->seconds = 0;
      }
      else {
        timeInputHeader = "[Update Time]";
        TimeValue t = TimeMgmt::getScheduleTime(timeSelectCursorPos);
        tmp_time->hours = t.hours;
        tmp_time->minutes = t.minutes;
        tmp_time->seconds = t.seconds;
      }
      timeAdjustCursorPos = 0;
      formPrevUi = TimeInputFallback::SetScheduleTimes;
      currentState = UiState::TimeInput;
      break;
    case UiButton::Menu:
      currentState = UiState::ScheduleMenu;
      break;
  }
}
static void SystemUI::RemoveTimesUi(UiButton i) {
  uint8_t scheduleSize = TimeMgmt::getScheduleSize();
  switch (i) {
    case UiButton::Up:
      if (timeSelectCursorPos > 0) {
        timeSelectCursorPos --;
      }
      break;
    case UiButton::Down:
      timeSelectCursorPos ++;
      if (timeSelectCursorPos >= scheduleSize - 1) {
        timeSelectCursorPos = scheduleSize - 1;
      }
      break;
    case UiButton::OK:
      TimeMgmt::removeScheduleTime(timeSelectCursorPos);
      timeSelectCursorPos = 0;
      break;
    case UiButton::Menu:
      currentState = UiState::ScheduleMenu;
      break;
  }
}
static void SystemUI::SetSysTimeUi(UiButton i) {
  switch (i) {
    case UiButton::Up:
      // Do nothing
      break;
    case UiButton::Down:
      // Do nothing
      break;
    case UiButton::OK:
      timeAdjustCursorPos = 0;
      formPrevUi = TimeInputFallback::SetSysTime;
      timeInputHeader = "[Set System Time]";
      tmp_time->hours = TimeMgmt::getHours();
      tmp_time->minutes = TimeMgmt::getMinutes();
      tmp_time->seconds = TimeMgmt::getSeconds();
      currentState = UiState::TimeInput;
      break;
    case UiButton::Menu:
      currentState = UiState::SystemMenu;
      break;
  }
} 
static void SystemUI::SysInfoUi(UiButton i) {
  switch (i) {
    case UiButton::Up:
      // Do nothing
      break;
    case UiButton::Down:
      // Do nothing
      break;
    case UiButton::OK:
      currentState = UiState::SystemMenu;
      break;
    case UiButton::Menu:
      currentState = UiState::SystemMenu;
      break;
  }
}
static void SystemUI::ResetUi(UiButton i) {
  switch (i) {
    case UiButton::Up:
      // Do nothing
      break;
    case UiButton::Down:
      // Do nothing
      break;
    case UiButton::OK:
      readyForReset = true;
      break;
    case UiButton::Menu:
      // Cancel reset
      if (!readyForReset) {
        currentState = UiState::SystemMenu;
        systemMenuCursorPos = 0;
      }
      break;
  }
}
static void SystemUI::TimeInputUi(UiButton i) {
  uint8_t tmp_h, tmp_m, tmp_s;
  switch (i) {
    case UiButton::Up:
      // Time format is 00:00:00
      // Cursor vals:   01 23 45
      switch (timeAdjustCursorPos) {
        case 0:
          // Hour 1st digit
          tmp_h = tmp_time->hours;
          tmp_time->hours = tmp_h % 10 + (((tmp_h / 10) + 1) % 3) * 10;
          break;
        case 1:
          // Hour 2nd digit
          tmp_h = tmp_time->hours;
          tmp_time->hours = (tmp_h / 10) * 10 + (tmp_h + 1) % (tmp_h >= 20 ? 4 : 10);
          break;
        case 2:
          // Minute 1st digit
          tmp_m = tmp_time->minutes;
          tmp_time->minutes = tmp_m % 10 + (((tmp_m / 10) + 1) % 6) * 10;
          break;
        case 3:
          // Minute 2nd digit
          tmp_m = tmp_time->minutes;
          tmp_time->minutes = (tmp_m / 10) * 10 + (tmp_m + 1) % 10;
          break;
        case 4:
          // Second 1st digit
          tmp_s = tmp_time->seconds;
          tmp_time->seconds = tmp_s % 10 + (((tmp_s / 10) + 1) % 6) * 10;
          break;
        case 5:
          // Second 2nd digit
          tmp_s = tmp_time->seconds;
          tmp_time->seconds = (tmp_s / 10) * 10 + (tmp_s + 1) % 10;
          break;
      }
      break;
    case UiButton::Down:
      // Time format is 00:00:00
      // Cursor vals:   01 23 45
      switch (timeAdjustCursorPos) {
        case 0:
          // Hour 1st digit
          tmp_h = tmp_time->hours;
          tmp_time->hours = tmp_h % 10 + (((tmp_h / 10) + 2) % 3) * 10;
          break;
        case 1:
          // Hour 2nd digit
          tmp_h = tmp_time->hours;
          tmp_time->hours = (tmp_h / 10) * 10 + (tmp_h + (tmp_h >= 20 ? 3 : 9)) % (tmp_h >= 20 ? 4 : 10);
          break;
        case 2:
          // Minute 1st digit
          tmp_m = tmp_time->minutes;
          tmp_time->minutes = tmp_m % 10 + (((tmp_m / 10) + 5) % 6) * 10;
          break;
        case 3:
          // Minute 2nd digit
          tmp_m = tmp_time->minutes;
          tmp_time->minutes = (tmp_m / 10) * 10 + (tmp_m + 9) % 10;
          break;
        case 4:
          // Second 1st digit
          tmp_s = tmp_time->seconds;
          tmp_time->seconds = tmp_s % 10 + (((tmp_s / 10) + 5) % 6) * 10;
          break;
        case 5:
          // Second 2nd digit
          tmp_s = tmp_time->seconds;
          tmp_time->seconds = (tmp_s / 10) * 10 + (tmp_s + 9) % 10;
          break;
      }
      break;
    case UiButton::OK:
      if (timeAdjustCursorPos != 5) {
        timeAdjustCursorPos++;

        // Special case: if 2 is input for first hour digit (hr >= 20), the second hour digit must be <= 3
        if (timeAdjustCursorPos == 1 && tmp_time->hours > 20 && tmp_time->hours % 10 > 3) {
          tmp_time->hours = 23;
        }
      }
      else {
        if (formPrevUi == TimeInputFallback::SetScheduleTimes) {
          // Update a schedule time (wherever the timeSelectCursorPos is valued at)
          uint8_t response = TimeMgmt::setScheduleTime(timeSelectCursorPos, tmp_time->hours, tmp_time->minutes, tmp_time->seconds);
          if (response != 1) {
            Debug::print("Set Schedule Error: ");
            Debug::println(String(response));
            SystemUI::SetText("[Error]\nFailed to set time.\nTimes must be apart\nby 60s or more.", 5);
          }
          currentState = UiState::SetTimes;
        }
        else if (formPrevUi == TimeInputFallback::SetSysTime) {
          // Update system time
          TimeMgmt::setHours(tmp_time->hours);
          TimeMgmt::setMinutes(tmp_time->minutes);
          TimeMgmt::setSeconds(tmp_time->seconds);
          currentState = UiState::SetTime;
        }
      }
      break;
    case UiButton::Menu:
      switch (formPrevUi) {
        case TimeInputFallback::SetScheduleTimes:
          currentState = UiState::SetTimes;
          break;
        case TimeInputFallback::SetSysTime:
          currentState = UiState::SetTime;
          break;
      }
      break;
  }
}
#pragma endregion Input_Handler_Methods

#pragma region LCD_Printing_Methods
static void SystemUI::UpdateUI() {
  // if the UI is paused, do nothing (early return)
  if (isPaused) return;
  // Otherwise,
  // Clear the current text from the screen.
  lcd.clear();
  lcd.setCursor(0, 0);
  // Depending on the currentState and variables, print certain text out.
  switch (currentState) {
    case UiState::Home:
      SystemUI::PrintHomeUi();
      break;
    case UiState::Menu:
      SystemUI::PrintMenuUi();
      break;
    case UiState::ScheduleMenu:
      SystemUI::PrintScheduleMenuUi();
      break;
    case UiState::SystemMenu:
      SystemUI::PrintSysMenuUi();
      break;
    case UiState::ViewTimes:
      SystemUI::PrintViewTimesUi();
      break;
    case UiState::SetTimes:
      SystemUI::PrintSetTimesUi();
      break;
    case UiState::RemoveTimes:
      SystemUI::PrintRemoveTimesUi();
      break;
    case UiState::SetTime:
      SystemUI::PrintSetSysTimeUi();
      break;
    case UiState::SystemInfo:
      SystemUI::PrintSysInfoUi();
      break;
    case UiState::Reset:
      SystemUI::PrintResetUi();
      break;
    case UiState::TimeInput:
      SystemUI::PrintTimeInputUi();
      break;
  }
}

static void SystemUI::PrintHomeUi() {
  lcd.print("Home");
  lcd.setCursor(0, 1);
  lcd.print(currentTime->toString());
}
static void SystemUI::PrintMenuUi() {
  lcd.print("[Main Menu]");
  lcd.setCursor(0, 1);
  lcd.print("Schedule Menu");
  if (mainMenuCursorPos == 0) {
    lcd.print(" <");
  }
  lcd.setCursor(0, 2);
  lcd.print("System Menu");
  if (mainMenuCursorPos == 1) {
    lcd.print(" <");
  }
  lcd.setCursor(0, 3);
  lcd.print("Back");
  if (mainMenuCursorPos == 2) {
    lcd.print(" <");
  }
}
static void SystemUI::PrintScheduleMenuUi() {
  lcd.print("[Schedule Menu]");
  // This menu has 4 choices, but only 3 can fit on a page.
  // Page 1
  if (scheduleMenuCursorPos < 3) {
    lcd.setCursor(0, 1);
    lcd.print("View times");
    if (scheduleMenuCursorPos == 0) {
      lcd.print(" <");
    }
    lcd.setCursor(0, 2);
    lcd.print("Set times");
    if (scheduleMenuCursorPos == 1) {
      lcd.print(" <");
    }
    lcd.setCursor(0, 3);
    lcd.print("Remove times");
    if (scheduleMenuCursorPos == 2) {
      lcd.print(" <");
    }
    lcd.setCursor(19, 3);
    lcd.write(1);
  }
  // Page 2
  else {
    lcd.setCursor(0, 1);
    lcd.print("Back");
    if (scheduleMenuCursorPos == 3) {
      lcd.print(" <");
    }
    lcd.setCursor(19, 1);
    lcd.write(0);
  }
}
static void SystemUI::PrintSysMenuUi() {
  lcd.print("[System Menu]");
  // This menu has 4 choices, but only 3 can fit on a page.
  // Page 1
  if (systemMenuCursorPos < 3) {
    lcd.setCursor(0, 1);
    lcd.print("Set time");
    if (systemMenuCursorPos == 0) {
      lcd.print(" <");
    }
    lcd.setCursor(0, 2);
    lcd.print("System info");
    if (systemMenuCursorPos == 1) {
      lcd.print(" <");
    }
    lcd.setCursor(0, 3);
    lcd.print("Reset");
    if (systemMenuCursorPos == 2) {
      lcd.print(" <");
    }
    lcd.setCursor(19, 3);
    lcd.write(1);
  }
  // Page 2
  else {
    lcd.setCursor(0, 1);
    lcd.print("Back");
    if (systemMenuCursorPos == 3) {
      lcd.print(" <");
    }
    lcd.setCursor(19, 1);
    lcd.write(0);
  }
}
static void SystemUI::PrintViewTimesUi() {
  lcd.print("[View Times]");
  uint8_t s = TimeMgmt::getScheduleSize();
  if (s == 0) {
    lcd.setCursor(0, 1);
    lcd.print("Schedule is empty.");
    return; // Early return.
  } // Else, schedule has nonzero # of times, so print/paginate them
  uint8_t group = timeSelectCursorPos / 3;
  uint8_t group_start_idx = group * 3;
  uint8_t cursor = 1;
  for (int i = group_start_idx; i < group_start_idx + 3; i++) {
    if (i < s) {
      lcd.setCursor(0, cursor);
      lcd.print(TimeMgmt::getScheduleTime(i).toString());
      if (i == timeSelectCursorPos) {
        lcd.print(" <");
      }
      cursor++;
    }
  }
  // Print up / down arrow and group
  // If size more than 3 (more than one page) and cursor is not on the last page,
  if (s > 3 && s - (group * 3) > 3) {
    // Print down arrow
    lcd.setCursor(19, 3);
    lcd.write(1);
  }
  // If size is more than 3 (more than one page) and cursor is not on first page,
  if (s > 3 && timeSelectCursorPos >= 3) {
    // Print up arrow
    lcd.setCursor(19, 1);
    lcd.write(0);
  }
  // Print page/group number
  lcd.setCursor(16, 2);
  lcd.print("Pg ");
  lcd.print(group + 1);
  
}
static void SystemUI::PrintSetTimesUi() {
  lcd.print("[Set Times]");
  uint8_t s = TimeMgmt::getScheduleSize();
  uint8_t cursor = 1;
  uint8_t group = timeSelectCursorPos / 3;
  uint8_t group_start_idx = group * 3;
  for (int i = group_start_idx; i < group_start_idx + 3; i++) {
    if (i < s) {
      lcd.setCursor(0, cursor++);
      lcd.print(TimeMgmt::getScheduleTime(i).toString());
      if (i == timeSelectCursorPos) {
        lcd.print(" <");
      }
    }
    // If schedule is not full, then also print an "add time" option
    if (s < 12 && i == s) {
      lcd.setCursor(0, cursor++);
      lcd.print("Add time");
      if (timeSelectCursorPos == i) {
        lcd.print(" <");
      }
    }
  }
  // Print up / down arrow and group
  // If size 3 or more (more than one page, including "add time" option) and cursor is not on the last page,
  if (s >= 3 && s - (group * 3) >= (s == 12 ? 4 : 3)) {
    // Print down arrow
    lcd.setCursor(19, 3);
    lcd.write(1);
  }
  // If size is 3 or more (more than one page, including "add time") and cursor is not on first page,
  if (s >= 3 && timeSelectCursorPos >= 3) {
    // Print up arrow
    lcd.setCursor(19, 1);
    lcd.write(0);
  }
  // Print page/group number
  lcd.setCursor(16, 2);
  lcd.print("Pg ");
  lcd.print(group + 1);
}
static void SystemUI::PrintRemoveTimesUi() {
  lcd.print("[Remove Times]");
  uint8_t s = TimeMgmt::getScheduleSize();
  if (s == 0) {
    lcd.setCursor(0, 1);
    lcd.print("Schedule is empty.");
    return; // Early return.
  } // Else, schedule has nonzero # of times, so print/paginate them
  uint8_t group = timeSelectCursorPos / 3;
  uint8_t group_start_idx = group * 3;
  uint8_t cursor = 1;
  for (int i = group_start_idx; i < group_start_idx + 3; i++) {
    if (i < s) {
      lcd.setCursor(0, cursor);
      lcd.print(TimeMgmt::getScheduleTime(i).toString());
      if (i == timeSelectCursorPos) {
        lcd.print(" <");
      }
      cursor++;
    }
  }
  // Print up / down arrow and group
  // If size more than 3 (more than one page) and cursor is not on the last page,
  if (s > 3 && s - (group * 3) > 3) {
    // Print down arrow
    lcd.setCursor(19, 3);
    lcd.write(1);
  }
  // If size is more than 3 (more than one page) and cursor is not on first page,
  if (s > 3 && timeSelectCursorPos >= 3) {
    // Print up arrow
    lcd.setCursor(19, 1);
    lcd.write(0);
  }
  // Print page/group number
  lcd.setCursor(16, 2);
  lcd.print("Pg ");
  lcd.print(group + 1);
}
static void SystemUI::PrintSetSysTimeUi() {
  lcd.print("[System Time]");
  lcd.setCursor(0, 1);
  lcd.print(currentTime->toString());
  lcd.setCursor(0, 2);
  lcd.print("Press OK to change");
}
static void SystemUI::PrintSysInfoUi() {
  lcd.print("[System Info]");
  lcd.setCursor(0, 1);
  lcd.print("Version: " + version);
  lcd.setCursor(0, 2);
  lcd.print("Time: ");
  lcd.print(currentTime->toString());
  if (debugEnabled) { // Configured during Init()
    lcd.setCursor(0, 3);
    lcd.print("Debug enabled.");
  }
}
static void SystemUI::PrintResetUi() {
  lcd.print("  ! RESET SYSTEM !");
  lcd.setCursor(0, 1);
  lcd.print("Are you sure?");
  lcd.setCursor(0, 2);
  lcd.print("OK = Confirm");
  lcd.setCursor(0, 3);
  lcd.print("MENU = Cancel");
}
static void SystemUI::PrintTimeInputUi() {
  lcd.print(timeInputHeader);
  lcd.setCursor(0, 1);
  lcd.print(tmp_time->toString());
  // Place the cursor below the digit being selected
  // 11:59:59
  // 01 34 67   <- col idx to set cursor to.
  lcd.setCursor(timeAdjustCursorPos + (timeAdjustCursorPos / 2), 2);
  // Print up arrow
  lcd.write(0);
}
#pragma endregion LCD_Printing_Methods
