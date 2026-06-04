#include "DeviceBoundContext.h"

#include <crypto/curves.h>
#include <crypto/hash_algo.h>
#include <drogon/drogon.h>
#include <utils/BinEncoding.h>
#include <utils/CryptoRandomizer.h>

#include <cstdint>
#include <span>

using namespace ikea400;
using namespace crypto;
using namespace curves;

DeviceBoundContext::DeviceBoundContext(std::span<const uint8_t> devicePublicKey)
    : m_devicePublicKey(PublicKey::fromDer(devicePublicKey)) {
  generateChallenge();
}

bool DeviceBoundContext::consumeChallenge(std::span<const uint8_t> proof) {
  bool success =
      verify(m_devicePublicKey, m_challenge.asSpan(), Signature{proof},
             SignatureAlgorithm::ecdsa(HashAlgo::Sha256));

  refreshChallenge();
  return success;
}

void DeviceBoundContext::generateChallenge() {
  m_randomizer.bytes(m_challenge);
}
