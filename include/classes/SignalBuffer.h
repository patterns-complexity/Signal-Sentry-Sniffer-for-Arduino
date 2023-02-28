#include <iostream>
#include <memory>
#include <deque>

using Buffer = std::deque<bool>;
using Timestamps = std::deque<uint32_t>;

class SignalBuffer {
  private:
    Buffer buffer;
    Timestamps timestamps;
    int buffer_size;

  public:
    SignalBuffer(uint32_t buffer_size);
    void addBit(bool value);
    bool getBit(int index);
    void addTimestamp(uint32_t timestamp);
    uint32_t getTimestamp(uint32_t index);
    Buffer getBuffer();
    Timestamps getTimestamps();
    int size();
};