#include <memory>
#include "init.h"


SignalBuffer signal_buffer = SignalBuffer(MAX_SIGNAL_LENGTH);
StateMachine state_machine = StateMachine();
SignalMemoryController smc = SignalMemoryController();

int signal_memory_index = 0;

void onSignalBitReceivedInterrupt() {
  if (state_machine.getCurrentState() != RECORDING) {
    return;
  }

  bool signal = digitalRead(PIN_SIGNAL_INPUT);
  signal_buffer.addBit(signal);
  signal_buffer.addTimestamp(micros());

  sendDebugMessage("Received signal bit: " + String(signal));
}

void onButton1Pressed() {
  State current_state = state_machine.getCurrentState();
  volatile bool pin_state = digitalRead(PIN_BUTTON_1);

  Serial.println("Button 1 pressed");

  if (pin_state == HIGH) {
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
}

void onButton2Pressed() {
  State current_state = state_machine.getCurrentState();
  volatile bool pin_state = digitalRead(PIN_BUTTON_2);

  Serial.println("Button 2 pressed");

  if (pin_state == HIGH) {
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
}

void saveSignal() {
  sendDebugMessage("Saving signal to memory");
  Buffer buffer = signal_buffer.getBuffer();
  Timestamps timestamps = signal_buffer.getTimestamps();
  sendDebugMessage("Signal buffer size: " + String(buffer.size()));
  smc.writeSignal(buffer, timestamps);
}

void loadSignal() {
  signal_buffer = smc.readSignal();
}

void sendSignal() {
  loadSignal();

  Timestamps temp_timestamps = Timestamps();
  Buffer temp_buffer = Buffer();

  const unsigned long first_timestamp = signal_buffer.getTimestamps()[0];

  String messages[signal_buffer.size()];

  for (int i = 0; i < signal_buffer.size(); i++) {
    bool signal = signal_buffer.getBit(i);
    unsigned long previous_timestamp = i == 0 ? first_timestamp : signal_buffer.getTimestamp(i - 1);
    unsigned long timestamp = signal_buffer.getTimestamp(i);
    unsigned long bit_length = timestamp - previous_timestamp;

    temp_buffer.push_back(signal);
    temp_timestamps.push_back(bit_length);
    messages[i] = String(i) + " | " + "Signal bit: " + String(signal) + " with length: " + String(bit_length);
  }

  for (int i = 0; i < temp_buffer.size(); i++) {
    bool signal = temp_buffer[i];
    unsigned long bit_length = temp_timestamps[i];

    wait(bit_length, true);
    digitalWrite(PIN_SIGNAL_OUTPUT, signal);

    sendDebugMessage(messages[i]);
  }

  state_machine.changeState(IDLE);
}

void setup() {
  // Init serial
  Serial.begin(115200);

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

  sendDebugMessage("Setup complete");
}

void loop() {
  State current_state = state_machine.getCurrentState();
  digitalWrite(PIN_BUTTON_1, HIGH);
  digitalWrite(PIN_BUTTON_2, HIGH);

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
      digitalWrite(PIN_SIGNAL_OUTPUT, HIGH);
      sendSignal();
      state_machine.changeState(IDLE);
      break;
    default:
      sendDebugMessage("Nothing to do here");
      break;
  }
}