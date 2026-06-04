#pragma once

#include <drogon/orm/DbClient.h>
#include <drogon/utils/coroutine.h>
#include <dto/vault_dto.h>
#include <utils/uuid.h>

#include <utility>
#include <vector>

#include "BaseDao.h"

namespace ikea400 {
namespace dao {

class VaultDao : public BaseDao<VaultDao> {
 public:
  VaultDao(drogon::orm::DbClientPtr dbClient) : BaseDao(std::move(dbClient)) {}

  drogon::Task<std::vector<dto::Vault>> getVaultsByUserId(const uuid& userId);
  drogon::Task<bool> addVault(const uuid& userId,
                              const ikea400::AddVaultRequest& request);

};
}  // namespace dao
}  // namespace ikea400