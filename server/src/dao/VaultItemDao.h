#pragma once
#include <drogon/utils/coroutine.h>
#include <dto/vault_item_dto.h>
#include <utils/uuid.h>

#include <optional>
#include <vector>

#include "BaseDao.h"

namespace ikea400 {
namespace dao {

class VaultItemDao : public BaseDao<VaultItemDao> {
 public:
  drogon::Task<std::vector<dto::VaultItemSummary>> getUserVaultItemsByVaultId(
      const uuid& userId, const uuid& vaultId);

  drogon::Task<bool> addVaultItem(const uuid& userId, const uuid& vaultId,
                                  const dto::AddVaultItemRequest& vaultItem);

  drogon::Task<bool> editVaultItem(const uuid& userId,
                                   const dto::EditVaultItemRequest& vaultItem);

  drogon::Task<std::optional<dto::VaultItem>> getVaultItemById(
      const uuid& userId, const uuid& itemId);
};
}  // namespace dao
}  // namespace ikea400