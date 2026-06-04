#include "Vault.h"

#include <crypto/kmac.h>
#include <dto/vault_dto.h>
#include <qdebug.h>
#include <qlogging.h>
#include <qstring.h>
#include <utils/BinEncoding.h>
#include <utils/ScopedTimer.h>
#include <utils/utils.h>
#include <utils/uuid.h>
#include <utils/CryptoRandomizer.h>
#include <utils/SecretRandomizer.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "MasterKeyManager.h"

using namespace ikea400;

Vault::Vault(const dto::Vault& vaultDto)
    : m_name(vaultDto.name),
      m_description(vaultDto.description),
      m_salt(vaultDto.salt.data),
      m_id(vaultDto.id),
      m_updatedAt(vaultDto.updated_at),
      m_createdAt(vaultDto.created_at),
      m_protectedKeyBlob(vaultDto.protected_key.data),
      m_vaultKey(),
      m_isDecrypted(false) {
  std::cout << "Loaded vault: " << m_name << " (ID: " << m_id.toString()
            << ")\n";
}

Vault::Vault(const MasterKeyManager* keyManager, const std::string& name,
             const std::string& description)
    : m_name(name),
      m_description(description),
      m_id(uuid::rand()),
      m_isDecrypted(true),
      m_updatedAt(utils::unix_seconds()),
      m_createdAt(utils::unix_seconds()),
      m_salt(16) {

  // Generate random values
  CryptoRandomizer randomizer;
  SecretRandomizer secretRandomizer;

  randomizer.bytes(m_salt);
  secretRandomizer.bytes(m_vaultKey);

  // Prepare the encrypted vault key
  m_protectedKeyBlob =
      keyManager->exportVaultKey(m_vaultKey.asArraySpan(), m_id);

  // Derive index key from salt and vault key
  crypto::kmac::derive(crypto::kmac::Variant::KMAC_256, m_vaultKey,
                       kIndexBindingKey, m_salt, m_indexKey);

  std::cout << bin::b64::encode(m_protectedKeyBlob) << "\n";
  std::cout << utils::estimateEntropy(m_protectedKeyBlob) << "\n";

  if (m_protectedKeyBlob.empty())
    throw std::runtime_error("Failed to encrypt vault key");
}

bool Vault::decryptVaultKey(const MasterKeyManager* keyManager) noexcept {
  if (m_isDecrypted) return true;

  auto vaultKeyOpt = keyManager->loadVaultKey(m_protectedKeyBlob, m_id);
  if (!vaultKeyOpt) {
    qWarning() << "Failed to decrypt vault key for vault: "
               << QString::fromStdString(m_name)
               << " (ID: " << QString::fromStdString(m_id.toString()) << ")\n";
    return false;
  }

  size_t len =
      crypto::kmac::derive<false>(crypto::kmac::Variant::KMAC_256, *vaultKeyOpt,
                                  kIndexBindingKey, m_salt, m_indexKey);
  if (len != m_indexKey.size()) {
    qWarning() << "Failed to derive index key for vault: "
               << QString::fromStdString(m_name)
               << " (ID: " << QString::fromStdString(m_id.toString()) << ")\n";
    return false;
  }

  // Useless to move because SecureArray is like std::array and in the stack and
  // trivial
  m_vaultKey = vaultKeyOpt.value();
  m_isDecrypted = true;
  return true;
}

std::optional<std::array<uint8_t, 32>> Vault::getItemNameHash(
    const std::string_view itemName) const noexcept {
  if (!m_isDecrypted) return std::nullopt;

  ScopedTimer timer("Vault::getItemNameHash");

  std::array<uint8_t, 32> hash;

  size_t len = crypto::kmac::derive<false>(crypto::kmac::Variant::KMAC_256, m_indexKey,
                              kItemNameHashBindingKey, itemName, hash);
  if (len != hash.size()) {
    qWarning() << "Failed to derive item name hash for vault: "
               << QString::fromStdString(m_name)
               << " (ID: " << QString::fromStdString(m_id.toString()) << ")\n";
    return std::nullopt;
  }

  return hash;
}
