#pragma once
#include <qmetatype.h>
#include <utils/uuid.h>

#include <cstdint>
#include <string>
#include <optional>

namespace ikea400::dto {

enum class VaultItemType : uint8_t { Login, Card };

Q_DECLARE_METATYPE(ikea400::dto::VaultItemType)

struct VaultItemMetadataDto {
  std::string name;
  int version = 1;
  VaultItemType type;
  bool is_deleted;
  bool is_favorite;
  std::optional<std::string> custom_icon;
};
}  // namespace ikea400::dto