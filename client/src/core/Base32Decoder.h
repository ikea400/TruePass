#include <QString>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

class Base32Decoder {
 private:
  static constexpr std::array<int8_t, 256> lookup = [] {
    std::array<int8_t, 256> table;
    table.fill(-1);
    for (int i = 0; i < 26; ++i) table['A' + i] = i;
    for (int i = 0; i < 6; ++i) table['2' + i] = 26 + i;
    return table;
  }();

 public:
  static std::vector<uint8_t> decode(const QString& input) {
    if (input.isEmpty()) return {};

    // Convert to Latin1 for efficient 8-bit access;
    // Base32 characters are ASCII-compatible.
    QByteArray ba = input.toLatin1();
    const uint8_t* data = reinterpret_cast<const uint8_t*>(ba.data());
    size_t len = ba.size();

    // Strip padding '=' for length calculation
    while (len > 0 && data[len - 1] == '=') {
      len--;
    }

    std::vector<uint8_t> output;
    output.reserve((len * 5) / 8);

    uint64_t buffer = 0;
    int bits_in_buffer = 0;

    for (size_t i = 0; i < len; ++i) {
      int8_t val = lookup[data[i]];
      if (val == -1) throw std::invalid_argument("Invalid Base32 character");

      buffer = (buffer << 5) | static_cast<uint64_t>(val);
      bits_in_buffer += 5;

      if (bits_in_buffer >= 8) {
        bits_in_buffer -= 8;
        output.push_back(
            static_cast<uint8_t>((buffer >> bits_in_buffer) & 0xFF));
      }
    }

    return output;
  }
};