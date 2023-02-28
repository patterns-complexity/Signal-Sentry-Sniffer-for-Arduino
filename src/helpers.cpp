#include "helpers.h"

void wait(uint32_t time, bool use_microseconds = false) {
  uint32_t start_time = use_microseconds ? micros() : millis();
  while (use_microseconds ? micros() - start_time < time : millis() - start_time < time) {};
}

void sendDebugMessage(String message) {
  if (DEBUG_MODE) {
    Serial.println(message);
  }
}

void sendMessage(String message) {
  if (COMMS_ENABLED) {
    Serial.println(message);
  }
}