#include <Arduino.h>
#include <EEPROM.h>

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#define PIN_SIGNAL_OUTPUT D6
#define PIN_LED D7
#define PIN_BUTTON_LISTEN D5
#define PIN_BUTTON_REPLAY D9
#define PIN_SIGNAL_INPUT D10
#define EMPTY_32_BITS (uint32_t) 0
#define MAX_SIGNAL_LENGTH 32
#define DEBUG_MODE true
#define COMMS_ENABLED true

#endif