#include <Arduino.h> // Arduino code environment
#include "TimeValue.h"
#include "Schedule.h"
#include "Debug.h"

Schedule::Schedule() {
  schedule = new TimeValue[12];
  count = 0;
}

// Sorts the times in the schedule, smallest time first, all empty times last
void Schedule::sort() {
  // Bubble sort is fine as our schedule has at max 12 entries.
  for (int i = 0; i < count; i++) {
    for (int j = 0; j < count - i - 1; j++) {
      if (schedule[j].totalSeconds() > schedule[j + 1].totalSeconds()) {
        // Swap elements
        TimeValue tmp = schedule[j];
        schedule[j] = schedule[j + 1];
        schedule[j + 1] = tmp;
      }
    }
  }

  Debug::println("Done sorting, the schedule is now: ");
  for (int i = 0; i < count; i++) {
    Debug::println(schedule[i].toString());
  }
}

TimeValue Schedule::getTime(uint8_t index) {
  return schedule[index];
}

// Returns true if no conflicts, or false if the time parameter (HH:MM:SS) has conflicts with any times in the schedule.
// Optionally skips checking at an index, for example if the time at index will be updated.
bool Schedule::checkTimeConflicts(uint8_t h, uint8_t m, uint8_t s, bool do_skip = false, uint8_t skip_index = 255) {
  long minimumTimeDiff = 60; // The number of seconds apart all times must be at minimum.
  long t1 = h, t2 = m, t3 = s; // Intentionally converting to Long to prevent overflowing. These are tmp vars.
  long timeAmnt = t1 * 3600 + t2 * 60 + t3; // The time parameter converted to total seconds.

  // For each time in the schedule,
  for (uint8_t i = 0; i < count; i++) {
    // If this time is actually being updated, we can skip checking it.
    if (do_skip && i == skip_index) {
      continue;
    }
    // Find the difference between total seconds of this time and the time parameter
    t3 = schedule[i].totalSeconds();
    t1 = abs(t3 - timeAmnt);
// Check for wrap-around conflicts (e.g. 23:59:30 and 0:00:01 are 31 secs apart, not 86369.)
    t2 = abs((t3 + 86400) % 86400 - timeAmnt);
    t3 = abs((timeAmnt + 86400) % 86400 - t3);

    // If that difference is less than minimumTimeDiff, return false (There is a time conflict.)
    if (t1 < minimumTimeDiff || t2 < minimumTimeDiff || t3 < minimumTimeDiff) {
      return false;
    }
  }
  
  return true; // No time conflicts.
}

// Adds a new time to the schedule if there is room. Returns true if successful.
// Returns: 1 = success, 2 = failed (time conlflict), 0 = failed (bad index)
uint8_t Schedule::addTime(uint8_t h, uint8_t m, uint8_t s) {
  // validation
  if (Schedule::count >= 12) {
    return 0;
  }
  if (!Schedule::checkTimeConflicts(h, m, s)) {
    return 2;
  }
  // Create a new time
  TimeValue* newTime = new TimeValue();
  newTime->hours = h;
  newTime->minutes = m;
  newTime->seconds = s;
  // Add time to the array
  schedule[count] = *newTime;
  count++;
  // Sort the schedule.
  Schedule::sort();
  return 1;
}

// Copies the H, M, S values from newTime to the time at position index.
// Returns: 1 = success, 2 = failed (time conlflict), 0 = failed (bad index)
uint8_t Schedule::updateTime(uint8_t index, uint8_t h, uint8_t m, uint8_t s) {
  // validation
  if (index < 0 || index >= Schedule::count) {
    return 0;
  }
  if (!Schedule::checkTimeConflicts(h, m, s, true, index)) {
    return 2;
  }
  // Update the time at this index
  schedule[index].hours = h;
  schedule[index].minutes = m;
  schedule[index].seconds = s;
  // Sort the schedule.
  Schedule::sort();
  return 1;
}

// Removes the time at position index from the schedule. Returns true if successful.
bool Schedule::removeTime(uint8_t index) {
  // validation
  if (count == 0 || index >= count) {
    return false;
  }
  // remove time
  TimeValue* newSchedule = new TimeValue[count - 1];
  uint8_t idx = 0;
  for (uint8_t i = 0; i < count; i++) {
    if (i != index) {
      newSchedule[idx] = schedule[i];
      idx++;
    }
  }
  delete[] schedule; // Free the old array
  schedule = newSchedule;
  count--;
  // Sorting is not necessary
  return true;
}


// Returns the count of times in the schedule
uint8_t Schedule::getCount() {
  return count;
}