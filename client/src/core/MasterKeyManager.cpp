#include "MasterKeyManager.h"

#include <crypto/kmac.h>
#include <opaque++.h>
#include <string.h>
#include <utils/ScopedTimer.h>
#include <utils/SecureArray.h>
#include <utils/uuid.h>

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "EnvelopeCodec.h"

using namespace ikea400;
using namespace ikea400::crypto;

bool MasterKeyManager::deriveMasterKey(
    const std::span<const uint8_t, opaque::kExportKeySize> exportKey,
    std::string_view username) noexcept {
  bool success = kmac::derive<false>(kmac::Variant::KMAC_256, exportKey,
                                     kMasterBindingKey, username, m_masterKey);
  if (!success) return false;

  m_hasMasterKey = true;
  return true;
}

bool MasterKeyManager::generateNewAccountKey() noexcept {
  m_hasAccountKey = false;
  if (!m_hasMasterKey) return false;
  try {
    m_secretRandomizer.bytes(m_accountKey);
    m_hasAccountKey = true;
    return true;
  } catch (...) {
    return false;
  }
}

std::vector<uint8_t> MasterKeyManager::exportMasterKey() const noexcept {
  if (!m_hasMasterKey || !m_hasAccountKey) return {};

  return EnvelopeCodec::encode(m_accountKey, m_masterKey,
                               EnvelopeCodec::Kdf::KMAC256);
}

bool MasterKeyManager::loadAccountKeyFromBlob(
    const std::span<const uint8_t> blobData) noexcept {
  std::span<const uint8_t> currBlob = blobData;

  ikea400::ScopedTimer timer("Load Account Key");
  std::vector<uint8_t> decodedAccountKey =
      EnvelopeCodec::decode(blobData, m_masterKey, EnvelopeCodec::Kdf::KMAC256);
  if (decodedAccountKey.size() == m_accountKey.size()) {
    std::memcpy(m_accountKey.data(), decodedAccountKey.data(),
                m_accountKey.size());
    m_hasAccountKey = true;
    return true;
  }
  return false;
}

std::optional<ikea400::SecureArray<uint8_t, 32>> MasterKeyManager::loadVaultKey(
    const std::span<const uint8_t> vaultProtectedKeyBlob,
    const ikea400::uuid& vaultId) const noexcept {
  if (!m_hasMasterKey || !m_hasAccountKey) return std::nullopt;

  ikea400::ScopedTimer timer("Load Vault Key");

  std::vector<uint8_t> decodedKey = EnvelopeCodec::decode(
      vaultProtectedKeyBlob, m_accountKey, kVaultBindingKey, vaultId.bytes());

  SecureArray<uint8_t, 32> vaultKey;
  if (decodedKey.size() == vaultKey.size()) {
    std::memcpy(vaultKey.data(), decodedKey.data(), vaultKey.size());
    return vaultKey;
  }
  return std::nullopt;
}

std::vector<uint8_t> MasterKeyManager::exportVaultKey(
    const std::span<const uint8_t, 32> vaultKey,
    const ikea400::uuid& vaultId) const noexcept {
  if (!m_hasMasterKey || !m_hasAccountKey) return {};

  return EnvelopeCodec::encode(vaultKey, m_accountKey, kVaultBindingKey,
                               vaultId.bytes());
}