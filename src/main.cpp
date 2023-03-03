#include <memory>
#include "init.h"

SignalBuffer signal_buffer = SignalBuffer(MAX_SIGNAL_LENGTH);
StateMachine state_machine = StateMachine();
SignalMemoryController smc = SignalMemoryController();

volatile State last_state;

void onSignalBitReceivedInterrupt() {
  if (state_machine.getCurrentState() != RECORDING) {
    return;
  }

  const bool signal = digitalRead(PIN_SIGNAL_INPUT);

  signal_buffer.addBit(signal);
  signal_buffer.addTimestamp(micros());

  sendDebugMessage("Received signal bit: " + String(signal));
}

void onButton1HeldFor5Seconds() {
  state_machine.changeState(IDLE);
  sendDebugMessage("Clearing EEPROM...");
  const SignalMemoryError sme = smc.clearMemory();
  if (sme != NO_ERROR) {
    sendDebugMessage("Error clearing EEPROM: " + String(sme));
    return;
  }
  sendDebugMessage("EEPROM cleared!");
}

void onButton1Pressed() {
  static unsigned long last_button_press = 0;

  const State current_state = state_machine.getCurrentState();
  const bool pin_state = digitalRead(PIN_BUTTON_1);

  if (pin_state == HIGH) {
    if (millis() - last_button_press > 5000) {
      onButton1HeldFor5Seconds();
    }
    return;
  } else if (millis() - last_button_press < 1000) {
    return;
  }

  switch (current_state) {
    case IDLE:
      sendDebugMessage("Switching state to RECORDING");
      state_machine.changeState(RECORDING);
      break;
    case RECORDING:
      sendDebugMessage("Switching state to SAVING");
      state_machine.changeState(SAVING);
      break;
    case SAVING:
      sendDebugMessage("Switching state to IDLE");
      state_machine.changeState(IDLE);
      break;
    default:
      sendDebugMessage("Nothing to do here");
      break;
  }

  last_button_press = millis();
}

void onButton2Pressed() {
  static unsigned long last_button_press = 0;

  const State current_state = state_machine.getCurrentState();
  const bool pin_state = digitalRead(PIN_BUTTON_2);

  if (pin_state == HIGH || millis() - last_button_press < 1000) {
    last_button_press = millis();
    return;
  }

  switch (current_state) {
    case IDLE:
      sendDebugMessage("Switching state to REPLAYING");
      state_machine.changeState(REPLAYING);
      break;
    case REPLAYING:
      sendDebugMessage("Switching state to IDLE");
      state_machine.changeState(IDLE);
      break;
    default:
      sendDebugMessage("Nothing to do here");
      break;
  }

  last_button_press = millis();
}

void saveSignal() {
  sendDebugMessage("Saving signal to memory");
  Buffer &buffer = signal_buffer.getBuffer();
  Timestamps &timestamps = signal_buffer.getTimestamps();
  sendDebugMessage("Signal buffer size: " + String(buffer.size()));
  const SignalMemoryError sme = smc.writeSignal(buffer, timestamps);
  if (sme != NO_ERROR) {
    sendDebugMessage("Error writing signal to memory: " + String(sme));
    return;
  }
  sendDebugMessage("Signal saved!");
}

void loadSignal() {
  Buffer &buffer = signal_buffer.getBuffer();
  Timestamps &timestamps = signal_buffer.getTimestamps();
  buffer.clear();
  timestamps.clear();
  const SignalMemoryError sme = smc.readSignal(buffer, timestamps);
  if (sme != NO_ERROR) {
    sendDebugMessage("Error reading signal from memory: " + String(sme));
    return;
  }
  sendDebugMessage("Signal loaded!");
}

void sendSignal() {
  sendDebugMessage("Loading signal from memory...");

  loadSignal();

  sendDebugMessage("Preparing signal...");

  sendDebugMessage("\n");

  Buffer &buffer = signal_buffer.getBuffer();
  Timestamps &timestamps = signal_buffer.getTimestamps();
  Timestamps &relative_timestamps = signal_buffer.getRelativeTimestamps();

  const size_t buffer_size = buffer.size();
  const size_t timestamps_size = timestamps.size();
  const size_t relative_timestamps_size = relative_timestamps.size();

  sendDebugMessage("Signal buffer size: " + String(buffer_size));
  sendDebugMessage("Absolute timestamps size: " + String(timestamps_size));
  sendDebugMessage("Relative timestamps size: " + String(relative_timestamps_size));

  sendDebugMessage("\n");

  sendDebugMessage("Preparation complete! Sending the signal... Size: " + String(buffer_size));
  for (int i = 0; i < buffer_size; i++) {
    wait(relative_timestamps[i], true);
    digitalWrite(PIN_SIGNAL_OUTPUT, buffer[i]);
    sendDebugMessage("Sent bit: "
      + String(i)
      + " | Relative delay: "
      + String(relative_timestamps[i])
      + " | "
      + String(timestamps[i])
    );
  }

  sendDebugMessage("Signal replay complete! Switching state to IDLE...");

  state_machine.changeState(IDLE);

  sendDebugMessage("Done!");
}

void setup() {
  // Init serial
  Serial.begin(115200);

  // Init EEPROM
  EEPROM.begin(EEPROM_INITIALIZED_SIZE);

  // Set outputs
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_SIGNAL_OUTPUT, OUTPUT);
  pinMode(PIN_BUTTON_1, OUTPUT);
  pinMode(PIN_BUTTON_2, OUTPUT);

  // Set inputs
  pinMode(PIN_SIGNAL_INPUT, INPUT);

  // Set pin values
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_SIGNAL_OUTPUT, LOW);

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(PIN_SIGNAL_INPUT), onSignalBitReceivedInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_1), onButton1Pressed, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_2), onButton2Pressed, CHANGE);

  sei();

  // Init state machine
  state_machine.changeState(IDLE);
  last_state = IDLE;

  sendDebugMessage("Setup complete");
}

void loop() {
  const State current_state = state_machine.getCurrentState();

  digitalWrite(PIN_BUTTON_1, HIGH);
  digitalWrite(PIN_BUTTON_2, HIGH);

  if (last_state == current_state) {
    return;
  }

  switch (current_state) {
    case IDLE:
      digitalWrite(PIN_LED, LOW);
      digitalWrite(PIN_SIGNAL_OUTPUT, LOW);
      break;
    case RECORDING:
      digitalWrite(PIN_LED, HIGH);
      digitalWrite(PIN_SIGNAL_OUTPUT, LOW);
      break;
    case SAVING:
      digitalWrite(PIN_LED, LOW);
      digitalWrite(PIN_SIGNAL_OUTPUT, LOW);
      saveSignal();
      state_machine.changeState(IDLE);
      break;
    case REPLAYING:
      digitalWrite(PIN_LED, HIGH);
      digitalWrite(PIN_SIGNAL_OUTPUT, LOW);
      sendDebugMessage("Replaying signal");
      sendSignal();
      sendDebugMessage("Replaying signal complete");
      state_machine.changeState(IDLE);
      break;
    default:
      sendDebugMessage("Nothing to do here");
      break;
  }

  last_state = current_state;
}