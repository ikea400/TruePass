#include "../../include/utils/CryptoRandomizer.h"

#include <openssl/err.h>
#include <openssl/rand.h>

#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

using namespace ikea400;

CryptoRandomizer::CryptoRandomizer() noexcept : CryptoRandomizer(RAND_bytes) {}

CryptoRandomizer::CryptoRandomizer(RandFunc* func) noexcept
    : m_randFunc(func) {}

void CryptoRandomizer::bytes(unsigned char* buffer, size_t length) {
  size_t offset = 0;
  constexpr size_t max_chunk =
      static_cast<size_t>(std::numeric_limits<int>::max());

  while (offset < length) {
    size_t remaining = length - offset;
    int chunkSize = static_cast<int>(std::min(remaining, max_chunk));

    if (m_randFunc(buffer + offset, chunkSize) != 1) {
      throw std::runtime_error("Failed to generate random bytes");
    }

    offset += chunkSize;
  }
}

uint64_t CryptoRandomizer::uniform(uint64_t range) {
  if (range < 2) return 0;

  // Smallest all-ones mask covering [0, range - 1]
  uint64_t mask = std::bit_ceil(range) - 1;

  // Handle edge case where bit_ceil might overflow if range is very large
  if (range > (1ULL << 63)) mask = std::numeric_limits<uint64_t>::max();

  uint64_t value{};

  do {
    bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

    value &= mask;

  } while (value >= range);

  return value;
}

uint32_t CryptoRandomizer::uniform(uint32_t range) {
  if (range < 2) return 0;

  // Smallest all-ones mask covering [0, range - 1]
  uint32_t mask = std::bit_ceil(range) - 1;

  // Handle edge case where bit_ceil might overflow if range is very large
  if (range > (1ULL << 31)) mask = std::numeric_limits<uint32_t>::max();

  uint32_t value{};

  do {
    bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

    value &= mask;

  } while (value >= range);

  return value;
}

int32_t CryptoRandomizer::uniform(int32_t min, int32_t max) {
  if (min > max) throw std::invalid_argument("min > max");
  if (min == max) return min;

  const uint64_t range = static_cast<uint64_t>(static_cast<int64_t>(max) -
                                               static_cast<int64_t>(min)) +
                         1ULL;

  const int64_t offset = static_cast<int64_t>(uniform(range));
  return static_cast<int32_t>(static_cast<int64_t>(min) + offset);
}
