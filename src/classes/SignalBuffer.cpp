#include "classes/SignalBuffer.h"

SignalBuffer::SignalBuffer(uint32_t buffer_size) {
  this->buffer_size = buffer_size;
}

void SignalBuffer::addBit(bool value) {
  this->buffer.push_back(value);
  if (this->buffer.size() > this->buffer_size) {
    this->buffer.pop_front();
  }
}

bool SignalBuffer::getBit(int index) {
  return this->buffer[index];
}

void SignalBuffer::addTimestamp(uint32_t timestamp) {
  this->timestamps.push_back(timestamp);
  if (this->timestamps.size() > this->buffer_size) {
    this->timestamps.pop_front();
  }
}

uint32_t SignalBuffer::getTimestamp(uint32_t index) {
  return this->timestamps[index];
}

Buffer &SignalBuffer::getBuffer() {
  return this->buffer;
}

Timestamps &SignalBuffer::getTimestamps() {
  return this->timestamps;
}

Timestamps &SignalBuffer::getRelativeTimestamps() {
  this->relative_timestamps.clear();
  Timestamps &timestamps = this->getTimestamps();
  for (int i = 0; i < timestamps.size(); i++) {
    if (i == 0) {
      this->relative_timestamps.push_back(0);
      continue;
    }
    this->relative_timestamps.push_back(timestamps[i] - timestamps[i - 1]);
  }
  return this->relative_timestamps;
}

int SignalBuffer::size() {
  return this->buffer.size();
}

