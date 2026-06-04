#pragma once
#include <cstdint>

#include "Randomizer.h"

namespace ikea400 {

class CryptoRandomizer : public Randomizer<CryptoRandomizer> {
 public:
  using Randomizer<CryptoRandomizer>::bytes;
  using Randomizer<CryptoRandomizer>::uniform;
  using Randomizer<CryptoRandomizer>::pick;
  using Randomizer<CryptoRandomizer>::shuffle;

  CryptoRandomizer() noexcept;

  void bytes(unsigned char* buffer, size_t length);
  uint64_t uniform(uint64_t range);
  uint32_t uniform(uint32_t range);
  int32_t uniform(int32_t min, int32_t max);

 protected:
  using RandFunc = int(unsigned char*, int);

  explicit CryptoRandomizer(RandFunc* func) noexcept;

  RandFunc* m_randFunc{nullptr};
};

}  // namespace ikea400