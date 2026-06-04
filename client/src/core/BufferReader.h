#pragma once
#include <string.h>
#include <utils/utils.h>

#include <cstdint>
#include <span>
#include <stdexcept>
#include <type_traits>

class BufferReader {
 public:
  explicit BufferReader(std::span<const uint8_t> data) : m_data(data) {}
  template <typename T>
    requires std::is_trivially_copyable_v<T>
  [[nodiscard]] T read() {
    if (m_data.size() < sizeof(T)) throw std::runtime_error("Buffer too small");
    T value{};
    std::memcpy(&value, m_data.data(), sizeof(T));
    m_data = m_data.subspan(sizeof(T));
    return ikea400::utils::big_to_native(value);
  }

  void readBytes(std::span<uint8_t> buffer) {
    if (m_data.size() < buffer.size())
      throw std::runtime_error("Buffer too small");
    std::memcpy(buffer.data(), m_data.data(), buffer.size());
    m_data = m_data.subspan(buffer.size());
  }

  [[nodiscard]] inline std::span<const uint8_t> readBytes(size_t lenght) {
    if (m_data.size() < lenght) throw std::runtime_error("Buffer too small");
    std::span<const uint8_t> result = m_data.first(lenght);
    m_data = m_data.subspan(lenght);
    return result;
  }

  [[nodiscard]] inline bool isEmpty() { return m_data.empty(); }

  [[nodiscard]] inline size_t remainingSize() { return m_data.size(); }

 private:
  std::span<const uint8_t> m_data;
};