#include "SystemUtil.h"

void (* setAddrTo0) (void) = 0; // Program counter set to 0 

void systemReset(int condition) {
  if (condition == 1) {
    setAddrTo0();
  }
}