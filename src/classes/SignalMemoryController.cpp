#include "classes/SignalMemoryController.h"

SignalMemoryController::SignalMemoryController() {}

void SignalMemoryController::writeSignal(Buffer buffer, Timestamps timestamps) {
  int signal_index = EEPROM_START_INDEX;

  uint32_t buffer_size = buffer.size()*sizeof(bool);
  uint32_t timestamps_size = timestamps.size()*sizeof(uint32_t);
  char eob = END_OF_BUFFER;
  char eot = END_OF_TIMESTAMPS;

  this->begin(buffer_size + timestamps_size + sizeof(eob) + sizeof(eot));

  for (int i = 0; i < buffer.size(); i++) {
    this->writeBool(signal_index + i, buffer[i]);
  }

  this->writeChar(signal_index + buffer_size, eob);

  signal_index += buffer_size + sizeof(eob);

  for (int i = 0; i < timestamps.size(); i++) {
    this->writeUInt(signal_index + (i * sizeof(uint32_t)), timestamps[i]);
  }

  this->writeChar(signal_index + timestamps_size, eot);

  this->commit();
}

SignalBuffer SignalMemoryController::readSignal() {
  int signal_index = EEPROM_START_INDEX;

  Buffer buffer;
  Timestamps timestamps;

  while (this->readChar(signal_index) != END_OF_BUFFER) {
    buffer.push_back(this->readBool(signal_index));
    signal_index++;
  }

  signal_index++;

  while (this->readChar(signal_index) != END_OF_TIMESTAMPS) {
    timestamps.push_back(this->readUInt(signal_index));
    signal_index += sizeof(uint32_t);
  }

  SignalBuffer signal_buffer = SignalBuffer(buffer.size());

  for (int i = 0; i < buffer.size(); i++) {
    signal_buffer.addBit(buffer[i]);
  }

  for (int i = 0; i < timestamps.size(); i++) {
    signal_buffer.addTimestamp(timestamps[i]);
  }

  return signal_buffer;
}