#include <Arduino.h> // Arduino code environment
#include "Debug.h"

static bool isInit = false; // Flipped to true in Init(). other methods interact with serial monitor if this is true.

static void Debug::Init() {
  Serial.begin(9600);
  isInit = true;
}

static void Debug::print(String msg) {
  if (!isInit) return;
  Serial.print(msg);
}

static void Debug::print(const char * msg) {
  if (!isInit) return;
  Serial.print(msg);
}

static void Debug::println(String msg) {
  if (!isInit) return;
  Serial.println(msg);
}

static void Debug::println(const char * msg) {
  if (!isInit) return;
  Serial.println(msg);
}

static bool Debug::isInputAvailable() {
  if (!isInit) return false;
  return Serial.available();
}

static int Debug::getIntInput() {
  if (!isInit) return -1;
  return Serial.parseInt();
}