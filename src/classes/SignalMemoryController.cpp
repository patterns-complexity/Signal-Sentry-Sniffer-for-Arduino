#include "classes/SignalMemoryController.h"

SignalMemoryController::SignalMemoryController() {};

SignalMemoryController::~SignalMemoryController() {};

SignalMemoryError SignalMemoryController::writeSignal(Buffer &buffer, Timestamps &timestamps) {
  // get the current index
  uint32_t start_index = EEPROM_START_INDEX;

  // define types sizes
  const uint8_t bool_size = sizeof(bool);
  const uint8_t uint32_t_size = sizeof(uint32_t);

  // get the length of the signal and timestamps
  const uint32_t signal_length_in_bytes = this->getSignalLengthInBytes(buffer);
  const uint32_t timestamps_length_in_bytes = this->getTimestampsLengthInBytes(timestamps);

  // calculate the space needed to write the signal and timestamps
  const size_t space_needed = signal_length_in_bytes + timestamps_length_in_bytes + (uint32_t_size * 2);

  sendDebugMessage("\n");

  sendDebugMessage("<- Signal length: " + String(signal_length_in_bytes));
  sendDebugMessage("<- Timestamps length: " + String(timestamps_length_in_bytes));
  sendDebugMessage("<- Space needed: " + String(space_needed));

  sendDebugMessage("\n");

  sendDebugMessage("Checking if there is enough space to write the signal and timestamps...");

  // check if there is enough space to write the signal and timestamps
  if (space_needed > EEPROM.length()) {
    sendDebugMessage("Not enough space to write signal and timestamps. Space needed: "
      + String(space_needed)
      + " bytes -> Memory size: "
      + String(EEPROM.length())
      + " bytes"
    );
    return CANT_ALLOCATE;
  }

  sendDebugMessage("Checking if the current index is valid...");

  // check if the current index is valid
  if (start_index > EEPROM.length()) {
    sendDebugMessage("Invalid current index");
    return CANT_ALLOCATE;
  }

  // write the signal length
  EEPROM.writeUInt(start_index, signal_length_in_bytes);

  // commit the write
  if (!EEPROM.commit()) {
    sendDebugMessage("Error writing signal length");
    return SIGNAL_WRITE_ERROR;
  }

  sendDebugMessage("Checking if the signal length was written correctly...");

  // check if the signal length was written correctly
  if (EEPROM.read(start_index) != signal_length_in_bytes) {
    sendDebugMessage("Error writing signal length");
    return SIGNAL_WRITE_ERROR;
  }

  // increment the index
  start_index += uint32_t_size;

  sendDebugMessage("Writing the timestamps' length to memory...");

  // write the timestamps length
  EEPROM.writeUInt(start_index, timestamps_length_in_bytes);

  // commit the write
  if (!EEPROM.commit()) {
    sendDebugMessage("Error writing timestamps length");
    return TIMESTAMP_WRITE_ERROR;
  }

  sendDebugMessage("Checking if the timestamps' length was written correctly...");

  // check if the timestamps length was written correctly
  if (EEPROM.readUInt(start_index) != timestamps_length_in_bytes) {
    sendDebugMessage("Error writing timestamps length");
    return TIMESTAMP_WRITE_ERROR;
  }

  // increment the index
  start_index += uint32_t_size;

  sendDebugMessage("\n");

  sendDebugMessage("Writing the signal to memory... Start index: " + String(start_index));
  sendDebugMessage("Signal length in bytes: " + String(signal_length_in_bytes));
  sendDebugMessage("Signal length in bool: " + String(signal_length_in_bytes / bool_size));

  sendDebugMessage("\n");

  // write the signal
  for (size_t i = 0; i < signal_length_in_bytes / bool_size; i++) {
    // write the signal value
    EEPROM.writeBool(start_index, buffer[i]);

    // increment the index
    start_index += bool_size;
  }

    // commit the write
  if (!EEPROM.commit()) {
    sendDebugMessage("Error writing signal");
    return SIGNAL_WRITE_ERROR;
  }

  sendDebugMessage("Checking if the signal was written correctly...");

  // check if the signal was written correctly
  for (size_t i = 0; i < signal_length_in_bytes / bool_size; i++) {
    // check if the signal value was written correctly
    if (EEPROM.readBool((start_index - signal_length_in_bytes) + i) != buffer[i]) {
      sendDebugMessage("Error writing signal");
      return SIGNAL_WRITE_ERROR;
    }
  }

  sendDebugMessage("\n");

  sendDebugMessage("Writing the timestamps to memory... Start index: " + String(start_index));
  sendDebugMessage("Timestamps length in bytes: " + String(timestamps_length_in_bytes));
  sendDebugMessage("Timestamps length in uint32_t: " + String(timestamps_length_in_bytes / uint32_t_size));

  sendDebugMessage("\n");

  // write the timestamps
  for (size_t i = 0; i < timestamps_length_in_bytes / uint32_t_size; i++) {
    // write the timestamp value
    EEPROM.writeUInt(start_index, timestamps[i]);

    // increment the index
    start_index += uint32_t_size;
  }

  // commit the write
  if (!EEPROM.commit()) {
    sendDebugMessage("Error writing timestamps");
    return TIMESTAMP_WRITE_ERROR;
  }

  sendDebugMessage("Checking if the timestamps were written correctly...");

  // check if the timestamps were written correctly
  for (size_t i = 0; i < timestamps_length_in_bytes / uint32_t_size; i++) {
    // check if the timestamp value was written correctly
    if (EEPROM.readUInt((start_index - timestamps_length_in_bytes) + (i * uint32_t_size)) != timestamps[i]) {
      sendDebugMessage("Error writing timestamps");
      return TIMESTAMP_WRITE_ERROR;
    }
  }

  sendDebugMessage("Writing successful! Final index: " + String(start_index));

  return NO_ERROR;
}

SignalMemoryError SignalMemoryController::readSignal(Buffer &buffer, Timestamps &timestamps) {
  // get the current index
  uint32_t start_index = EEPROM_START_INDEX;

  // define types sizes
  const uint32_t bool_size = sizeof(bool);
  const uint32_t uint32_t_size = sizeof(uint32_t);

  sendDebugMessage("Checking if the current index is valid...");

  // check if the current index is valid
  if (start_index > EEPROM.length()) {
    sendDebugMessage("Invalid current index");
    return CANT_ALLOCATE;
  }

  sendDebugMessage("Reading the signal length...");

  // read the signal length
  const uint32_t signal_length_in_bytes = EEPROM.readUInt(start_index);

  sendDebugMessage("Signal length in bytes: " + String(signal_length_in_bytes));

  // increment the index
  start_index += uint32_t_size;

  sendDebugMessage("Reading the timestamps' length...");

  // read the timestamps length
  const uint32_t timestamps_length_in_bytes = EEPROM.readUInt(start_index);

  sendDebugMessage("Timestamps length in bytes: " + String(timestamps_length_in_bytes));

  // increment the index
  start_index += uint32_t_size;

  sendDebugMessage("\n");

  sendDebugMessage("Reading the signal... Start index: " + String(start_index));
  sendDebugMessage("Signal length in bytes: " + String(signal_length_in_bytes));
  sendDebugMessage("Signal length in bool: " + String(signal_length_in_bytes / bool_size));

  sendDebugMessage("\n");

  // read the signal
  for (size_t i = 0; i < signal_length_in_bytes / bool_size; i++) {
    // read the signal value
    buffer.push_back(EEPROM.readBool(start_index));

    // increment the index
    start_index += bool_size;
  }

  sendDebugMessage("Reading the timestamps... Start index: " + String(start_index));
  sendDebugMessage("Timestamps length in bytes: " + String(timestamps_length_in_bytes));
  sendDebugMessage("Timestamps length in uint32_t: " + String(timestamps_length_in_bytes / uint32_t_size));

  sendDebugMessage("\n");

  // read the timestamps
  for (size_t i = 0; i < timestamps_length_in_bytes / uint32_t_size; i++) {
    // read the timestamp value
    timestamps.push_back(EEPROM.readUInt(start_index));

    // increment the index
    start_index += uint32_t_size;
  }

  sendDebugMessage("Reading successful! Final index: " + String(start_index));

  return NO_ERROR;
}

SignalMemoryError SignalMemoryController::clearMemory() {
  // get the current index
  uint32_t start_index = EEPROM_START_INDEX;

  sendDebugMessage("Checking if the current index is valid...");

  // check if the current index is valid
  if (start_index > EEPROM.length()) {
    sendDebugMessage("Invalid current index");
    return CANT_ALLOCATE;
  }

  sendDebugMessage("Clearing the memory...");

  // clear the memory
  for (size_t i = start_index; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }

  // commit the write
  if (!EEPROM.commit()) {
    sendDebugMessage("Error clearing the memory");
    return MEMORY_CLEAR_ERROR;
  }

  sendDebugMessage("Memory cleared!");

  return NO_ERROR;
}

uint32_t SignalMemoryController::getSignalLengthInBytes(Buffer &buffer) {
  return buffer.size() * sizeof(bool);
}

uint32_t SignalMemoryController::getTimestampsLengthInBytes(Timestamps &timestamps) {
  return timestamps.size() * sizeof(uint32_t);
}