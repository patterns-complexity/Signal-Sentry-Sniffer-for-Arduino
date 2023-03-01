#include "settings.h"
#include "classes/SignalBuffer.h"
#include <EEPROM.h>
#include <memory>

class SignalMemoryController : EEPROMClass {
  public:
    SignalMemoryController();
    int getFirstEmptyIndex();
    void writeSignal(Buffer buffer, Timestamps timestamps);
    SignalBuffer readSignal();
};
