#pragma once
#include <ctime>
#include <string>
#include <vector>

#include "../utils/BinVector.h"
#include "../utils/uuid.h"

namespace ikea400 {
namespace dto {
struct Vault {
  uuid id;
  std::string name;
  std::string description;
  BinVector<63, 128> protected_key;
  BinVector<16, 32> salt;
  std::int64_t updated_at;
  std::int64_t created_at;
};
}  // namespace dto

struct AddVaultRequest {
  uuid id;
  std::string name;
  std::string description;
  BinVector<63, 128> protected_key;
  BinVector<16, 32> salt;
};

}  // namespace ikea400