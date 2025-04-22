#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

class Debug {
  public:
    static void Init();
    static void print(String msg);
    static void print(const char msg[]);
    static void println(String msg);
    static void println(const char msg[]);
    static bool isInputAvailable();
    static int getIntInput();
};

#endif