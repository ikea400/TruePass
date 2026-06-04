#pragma once

#include <string>

#include "../utils/uuid.h"

namespace ikea400::dto {
struct UserProfile {
  uuid userId;
  std::string username;
  std::string email;
};
}  // namespace ikea400::dto
