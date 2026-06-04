#pragma once

#include <crypto/curves.h>
#include <utils/CryptoRandomizer.h>
#include <utils/SecureArray.h>

#include <cstdint>
#include <span>

namespace ikea400 {
class DeviceBoundContext {
 public:
  DeviceBoundContext(std::span<const uint8_t> devicePublicKey);

  const SecureArray<uint8_t, 32>& getChallenge() const noexcept {
    return m_challenge;
  }

  bool consumeChallenge(std::span<const uint8_t> proof);

  void refreshChallenge() { generateChallenge(); }

 private:
  void generateChallenge();

 private:
  crypto::curves::PublicKey m_devicePublicKey;
  SecureArray<uint8_t, 32> m_challenge;
  CryptoRandomizer m_randomizer;
};
}  // namespace ikea400