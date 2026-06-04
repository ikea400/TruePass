#pragma once
#include <cstdint>
#include <span>
#include <vector>

class BufferWriter {
 public:
  BufferWriter(std::vector<uint8_t>& buffer) : buffer_(buffer) {}

  void write(const uint8_t* src, size_t length) {
    if (src == nullptr || length == 0) return;
    buffer_.insert(buffer_.end(), src, src + length);
  }

  void write(const std::span<const uint8_t> data) {
    if (data.empty()) return;
    write(data.data(), data.size());
  }

  void write(uint8_t value) { buffer_.push_back(value); }

 private:
  std::vector<uint8_t>& buffer_;
};
