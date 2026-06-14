#pragma once

#include "VaultItemDetailsBase.h"

namespace ikea400::dto {
struct IdentityItemDto {
  VaultItemDetailsBase base;
  std::string first_name;
  std::string last_name;
  std::string address;
  std::string email;
  std::string date_of_birth;
  std::string username;
};
}  // namespace ikea400::dto