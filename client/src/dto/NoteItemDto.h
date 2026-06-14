#pragma once
#include <string>
#include <vector>
#include <variant>
#include <cstdint>

#include "VaultItemDetailsBase.h"

namespace ikea400::dto {

enum class FieldTypeDto : uint8_t {
  Text,
  Password,
  Boolean
};

struct CustomFieldDto {
  std::string name;
  FieldTypeDto type;
  std::variant<std::string, bool> value;
};

struct NoteItemDto {
  VaultItemDetailsBase base;
  std::string note_content;
  std::vector<CustomFieldDto> custom_fields;
};

}  // namespace ikea400::dto
