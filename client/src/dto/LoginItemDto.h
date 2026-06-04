#pragma once
#include <string>

#include "VaultItemDetailsBase.h"

namespace ikea400::dto {
struct LoginItemDto {
  VaultItemDetailsBase base;
  std::string username;
  std::string email;
  std::string password;
  std::string totp_secret;
  std::string website;
};
}  // namespace ikea400::dto