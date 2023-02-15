#include <Arduino.h>
#include "settings.hpp"

const byte STATE_IDLE = 0b00000000;
const byte STATE_RECORDING = 0b00000001;
const byte STATE_SAVING = 0b00000010;
const byte STATE_REPLAYING = 0b00000011;

const byte REPEAT_6 = 6;

const bool USE_MICROS = true;

volatile byte state = STATE_IDLE;
volatile uint32_t state_change_counter = 0;

volatile unsigned long last_played_interval_value = 0;

struct Signal {
  volatile uint32_t signal_data = EMPTY_32_BITS;
  volatile byte bit_index = 0;
  volatile uint16_t intervals[MAX_SIGNAL_LENGTH];
};

Signal global_signal = Signal();

void reset() {
  global_signal.signal_data = EMPTY_32_BITS;
  for (int i = 0; i < MAX_SIGNAL_LENGTH; i++) {
    global_signal.intervals[i] = 0;
  }
  global_signal.bit_index = 0;
  state = STATE_IDLE;
}

void printToSerialPort(String message) {
  if (DEBUG_MODE) { Serial.println(message); }
}

void printCommsToSerialPort(String message) {
  if (COMMS_ENABLED) { Serial.println("!comms: " + message); }
}

volatile bool millisHavePassed(unsigned long start_time, unsigned long interval) { return (millis() - start_time) >= interval; }
volatile bool microsHavePassed(unsigned long start_time, unsigned long interval) { return (micros() - start_time) >= interval; }

void wait(unsigned long interval, bool use_micros = false) {
  unsigned long start_time = use_micros ? micros() : millis();
  while (use_micros ? !microsHavePassed(start_time, interval) : !millisHavePassed(start_time, interval)) {}
}

unsigned long timeSince(unsigned long start_time, bool use_micros = false) {
  return use_micros ? micros() - start_time : millis() - start_time;
}

void changeState(byte new_state) {
  static byte last_state = STATE_IDLE;

  if (new_state == state) { return; }

  last_state = state;
  state = new_state;
  state_change_counter++;
}

void listenButtonPressed() {
  if (state != STATE_IDLE && state != STATE_RECORDING) { return; }

  if (digitalRead(PIN_BUTTON_LISTEN) == LOW) {
    switch (state) {
      case STATE_IDLE:
        printToSerialPort("Recording..." + String(state));
        changeState(STATE_RECORDING);
        break;
      case STATE_RECORDING:
        printToSerialPort("Saving..." + String(state));
        changeState(STATE_SAVING);
        break;
    }
  }
}

void replayButtonPressed() {
  if (state != STATE_IDLE) { return; }

  if (digitalRead(PIN_BUTTON_REPLAY) == LOW) {
    printToSerialPort("Replay button pressed" + String(state) + " - " + String(state_change_counter));

    changeState(STATE_REPLAYING);
  }
}

void signalReceived() {
  static unsigned long last_signal_time = 0;
  static uint32_t local_signal = EMPTY_32_BITS;
  static uint16_t local_intervals[MAX_SIGNAL_LENGTH];
  static byte local_bit_index = 0;
  static byte last_state_change_counter = 0;

  // If the state has changed, reset the local signal
  if (last_state_change_counter != state_change_counter) {
    printToSerialPort("Resetting local signal");
    local_signal = EMPTY_32_BITS;
    last_signal_time = 0;
    local_bit_index = 0;
    for (int i = 0; i < MAX_SIGNAL_LENGTH; i++) {
      local_intervals[i] = 0;
    }
    last_state_change_counter = state_change_counter;
  }

  if (state != STATE_RECORDING || local_bit_index == MAX_SIGNAL_LENGTH) { return; }

  // Update the local interval for the current bit
  local_intervals[local_bit_index] = last_signal_time == 0
    ? 0
    : timeSince(last_signal_time, USE_MICROS);

  // Update the last signal time
  last_signal_time = micros();

  // Update the local signal data
  if (digitalRead(PIN_SIGNAL_INPUT) == HIGH) {
    local_signal |= (1 << local_bit_index);
  } else {
    local_signal &= ~(1 << local_bit_index);
  }

  bool current_bit_value = (local_signal >> local_bit_index) & 1;
  printToSerialPort(
    "Signal received: "
      + String(current_bit_value)
      + " - "
      + String(local_intervals[local_bit_index])
      + " - (" + String(local_bit_index) + ")"
  );

  // Increment the local bit index
  local_bit_index++;

  // Update the global signal
  global_signal.signal_data = local_signal;
  global_signal.bit_index = local_bit_index;
  for (int i = 0; i < MAX_SIGNAL_LENGTH; i++) {
    global_signal.intervals[i] = local_intervals[i];
  }
}

void saveSignalToEEPROM() {
  if (state != STATE_SAVING) { return; }

  EEPROM.writeUInt(0, global_signal.signal_data);

  for (int i = 0; i < MAX_SIGNAL_LENGTH; i++) {
    EEPROM.writeShort(
      ( ( (int) MAX_SIGNAL_LENGTH ) / 8 )
      + (i*2),
      global_signal.intervals[i]
    );
  }

  EEPROM.commit();
}

void loadSignalFromEEPROM() {
  if (state != STATE_REPLAYING) { return; }

  global_signal.signal_data = EEPROM.readUInt(0);

  for (int i = 0; i < MAX_SIGNAL_LENGTH; i++) {
    global_signal.intervals[i] = EEPROM.readShort(
      ( ( (int) MAX_SIGNAL_LENGTH ) / 8 )
      + (i*2)
    );
  }
}

void playSignal() {
  if (state != STATE_REPLAYING) { return; }

  for (int i = 0; i < MAX_SIGNAL_LENGTH; i++) {
    bool current_bit_value = (global_signal.signal_data >> i) & 1;
    wait(global_signal.intervals[i], USE_MICROS);
    digitalWrite(PIN_SIGNAL_OUTPUT, current_bit_value);
    volatile unsigned long current_time = micros();
    volatile unsigned long timeDiff = i == 0 ? 0 : current_time - last_played_interval_value;
    printToSerialPort(
      "Signal received: "
        + String(current_bit_value)
        + " - "
        + String(global_signal.intervals[i])
        + " - (" + String(i) + ") - "
        + "Actual interval: " + String(timeDiff)
    );
    printCommsToSerialPort(String(current_bit_value) + ", " + String(global_signal.intervals[i]));
    last_played_interval_value = current_time;
    if (i == MAX_SIGNAL_LENGTH - 1) {
      wait(100);
    }
  }
  last_played_interval_value = 0;
  digitalWrite(PIN_SIGNAL_OUTPUT, LOW);
}

void replaySignal(byte repeat_times = 1) {
  if (state != STATE_REPLAYING) { return; }

  for (int i = 0; i < repeat_times; i++) {
    playSignal();
  }
}

void blinkLED(byte times, unsigned long timeLit = 200, unsigned long timeUnlit = 200, bool use_micros = false) {
  for (int i = 0; i < times; i++) {
    digitalWrite(PIN_LED, HIGH);
    wait(timeLit, use_micros);
    digitalWrite(PIN_LED, LOW);
    wait(timeUnlit, use_micros);
  }
}

// The setup() method runs once, when the sketch starts
void setup () {
  // Initialize the serial port
  Serial.begin(9600);

  // Initialize EEPROM dialogue with uint32_t and (uint16_t * MAX_SIGNAL_LENGTH) worth of memory
  EEPROM.begin(
    ( ( (int) MAX_SIGNAL_LENGTH ) / 8 )
      + (2 * MAX_SIGNAL_LENGTH)
  );

  // Initialize signal data
  global_signal.signal_data = EMPTY_32_BITS;

  // Initialize intervals array
  for (int i = 0; i < MAX_SIGNAL_LENGTH; i++) {
    global_signal.intervals[i] = 0;
  }

  // Initialize the bit index
  global_signal.bit_index = 0;

  // Set pinModes
  pinMode(PIN_SIGNAL_OUTPUT, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_SIGNAL_INPUT, INPUT);
  pinMode(PIN_BUTTON_LISTEN, OUTPUT);
  pinMode(PIN_BUTTON_REPLAY, OUTPUT);

  // Set initial states
  digitalWrite(PIN_SIGNAL_OUTPUT, LOW);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_BUTTON_LISTEN, HIGH);
  digitalWrite(PIN_BUTTON_REPLAY, HIGH);

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(PIN_SIGNAL_INPUT), signalReceived, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_LISTEN), listenButtonPressed, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_REPLAY), replayButtonPressed, CHANGE);
}

// the loop() method runs over and over again,
void loop () {
  // Handle state changes
  static byte last_state = 0;

  if (last_state != state) {
    printToSerialPort("State: " + String(last_state) + " -> " + String(state) + " ");
    last_state = state;
  }

  switch (state) {
    case STATE_IDLE:
      digitalWrite(PIN_LED, LOW);
      break;
    case STATE_RECORDING:
      digitalWrite(PIN_LED, HIGH);
      break;
    case STATE_SAVING:
      blinkLED(2, 250, 250);
      saveSignalToEEPROM();
      changeState(STATE_IDLE);
      break;
    case STATE_REPLAYING:
      blinkLED(6, 15, 15);
      loadSignalFromEEPROM();
      replaySignal(REPEAT_6);
      changeState(STATE_IDLE);
      break;
  }
}