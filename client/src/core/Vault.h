#pragma once
#include <corecrt.h>
#include <dto/vault_dto.h>
#include <utils/SecureArray.h>
#include <utils/uuid.h>

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

class MasterKeyManager;

class Vault {
 public:
  explicit Vault(const ikea400::dto::Vault& vaultDto);

  Vault(const MasterKeyManager* keyManager, const std::string& name,
        const std::string& description);

  const std::string& getName() const noexcept { return m_name; }
  const std::string& getDescription() const noexcept { return m_description; }

  ikea400::uuid getId() const noexcept { return m_id; }

  std::span<const uint8_t> getSalt() const noexcept { return m_salt; }
  std::span<const uint8_t> getProtectedKeyBlob() const noexcept {
    return m_protectedKeyBlob;
  }

  std::span<const uint8_t, 32> getVaultKey() const {
    if (!m_isDecrypted) {
      throw std::runtime_error("Vault key is not decrypted");
    }
    return m_vaultKey.asArraySpan();
  }

  bool isDecrypted() const noexcept { return m_isDecrypted; }

  bool decryptVaultKey(const MasterKeyManager* keyManager) noexcept;

  std::optional<std::array<uint8_t, 32>> getItemNameHash(
      const std::string_view itemName) const noexcept;

 private:
  std::string m_name;
  std::string m_description;
  std::vector<uint8_t> m_salt;
  std::vector<uint8_t> m_protectedKeyBlob;
  ikea400::SecureArray<uint8_t, 32> m_vaultKey;
  ikea400::SecureArray<uint8_t, 32> m_indexKey;
  ikea400::uuid m_id;
  std::int64_t m_updatedAt;
  std::int64_t m_createdAt;
  bool m_isDecrypted;

  static inline constexpr std::string_view kIndexBindingKey =
      "TruePass_v1_IndexKey";
  static inline constexpr std::string_view kItemNameHashBindingKey =
      "TruePass_v1_ItemNameHash";
};