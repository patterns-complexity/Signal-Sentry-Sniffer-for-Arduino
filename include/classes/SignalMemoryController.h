#include "settings.h"
#include "helpers.h"
#include "classes/SignalBuffer.h"
#include <EEPROM.h>
#include <memory>

enum SignalMemoryError {
  NO_ERROR,
  CANT_ALLOCATE,
  SIGNAL_WRITE_ERROR,
  TIMESTAMP_WRITE_ERROR,
  MEMORY_CLEAR_ERROR
};

class SignalMemoryController {
  public:
    SignalMemoryController();
    ~SignalMemoryController();
    SignalMemoryError writeSignal(Buffer &buffer, Timestamps &timestamps);
    SignalMemoryError readSignal(Buffer &buffer, Timestamps &timestamps);
    SignalMemoryError clearMemory();
    uint32_t getSignalLengthInBytes(Buffer &buffer);
    uint32_t getTimestampsLengthInBytes(Timestamps &timestamps);
};
