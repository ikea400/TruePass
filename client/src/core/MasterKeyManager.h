#pragma once
#include <opaque++.h>
#include <utils/SecureArray.h>
#include <utils/uuid.h>
#include <utils/SecretRandomizer.h>

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

class MasterKeyManager {
 public:
  bool deriveMasterKey(
      const std::span<const uint8_t, opaque::kExportKeySize> exportKey,
      std::string_view username) noexcept;
  bool generateNewAccountKey() noexcept;

  std::vector<uint8_t> exportMasterKey() const noexcept;

  bool loadAccountKeyFromBlob(const std::span<const uint8_t> blobData) noexcept;

  std::optional<ikea400::SecureArray<uint8_t, 32>> loadVaultKey(
      const std::span<const uint8_t> vaultProtectedKeyBlob,
      const ikea400::uuid& vaultId) const noexcept;

  std::vector<uint8_t> exportVaultKey(
      const std::span<const uint8_t, 32> vaultKey,
      const ikea400::uuid& vaultId) const noexcept;

 private:
  ikea400::SecureArray<uint8_t, 32> m_masterKey{};
  ikea400::SecureArray<uint8_t, 32> m_accountKey{};
  ikea400::SecretRandomizer m_secretRandomizer{};

  bool m_hasMasterKey = false;
  bool m_hasAccountKey = false;

  constexpr inline static std::string_view kMasterBindingKey =
      "TruePass_v1_MasterKey";
  constexpr inline static std::string_view kVaultBindingKey =
      "TruePass_v1_VaultKey";
};