#pragma once
#include <dto/vault_item_dto.h>
#include <utils/uuid.h>

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "../dto/VaultItemMetadataDto.h"
#include "../model/VaultItemModel.h"

class Vault;
class VaultItem {
 public:
  using VaultItemType = ikea400::dto::VaultItemType;

  static VaultItem createFromModel(const VaultItemModel& model,
                                   const Vault& vault);

  static VaultItem loadFromSummary(
      const ikea400::dto::VaultItemSummary& summary, const Vault& vault);

  [[nodiscard]] ikea400::uuid getId() const noexcept { return m_id; }
  [[nodiscard]] ikea400::uuid getVaultId() const noexcept { return m_vaultId; }
  [[nodiscard]] const std::string& getName() const noexcept { return m_name; }
  [[nodiscard]] const std::string& getCustomIcon() const noexcept {
    return m_customIcon;
  }
  [[nodiscard]] VaultItemType getType() const noexcept { return m_type; }
  [[nodiscard]] bool isFavorite() const noexcept { return m_isFavorite; }
  [[nodiscard]] bool isDeleted() const noexcept { return m_isDeleted; }
  [[nodiscard]] std::int64_t getCreatedAt() const noexcept {
    return m_createdAt;
  }
  [[nodiscard]] std::int64_t getUpdatedAt() const noexcept {
    return m_updatedAt;
  }

  [[nodiscard]] std::span<const uint8_t> getEncryptedMetadata() const noexcept {
    return m_encrypted_metadata;
  }
  [[nodiscard]] std::span<const uint8_t> getEncryptedData() const noexcept {
    return m_encrypted_data;
  }

  [[nodiscard]] bool hasData() const noexcept { return m_hasData; }

  void setEncryptedData(std::span<const uint8_t> encryptedData) {
    m_encrypted_data.assign_range(encryptedData);
    m_hasData = true;
  }

  void setFavorite(bool isFavorite) { m_isFavorite = isFavorite; }

  [[nodiscard]] VaultItemModel toModel(const Vault& vault) const;

 private:
  std::string m_name;
  std::string m_customIcon;
  std::vector<uint8_t> m_encrypted_metadata;
  std::vector<uint8_t> m_encrypted_data;
  ikea400::uuid m_id;
  ikea400::uuid m_vaultId;
  std::int64_t m_createdAt;
  std::int64_t m_updatedAt;
  VaultItemType m_type;
  bool m_isDeleted{false};
  bool m_isFavorite{false};
  bool m_hasData{false};

  static inline constexpr std::string_view kVaultItemDataBindingKey =
      "TruePass_v1_VaultItemData";
  static inline constexpr std::string_view kVaultItemMetadataBindingKey =
      "TruePass_v1_VaultItemMetadata";
};