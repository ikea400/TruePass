#pragma once
#include "../utils/BinArray.h"
#include "../utils/BinVector.h"
#include "../utils/uuid.h"

namespace ikea400 {
namespace dto {

struct VaultItemSummary {
  uuid id;
  uuid vaultId;
  BinVector<0, 65535> protectedMetaData;
  std::int64_t createdAt{};
};

struct VaultItem {
  uuid id;
  uuid vaultId;
  BinVector<0, 65535> protected_metadata;
  BinVector<0, 65535> protected_data;
  std::int64_t created_at{};
  std::int64_t updated_at{};
};

struct AddVaultItemRequest {
  uuid id;
  uuid vaultId;
  BinVector<0, 65535> protected_metadata;
  BinVector<0, 65535> protected_data;
  BinArray<32> name_hash;
};

using EditVaultItemRequest = AddVaultItemRequest;

}  // namespace dto
}  // namespace ikea400