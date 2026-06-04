#include "VaultDao.h"

#include <corecrt.h>
#include <drogon/orm/Result.h>
#include <drogon/utils/coroutine.h>
#include <dto/vault_dto.h>
#include <utils/uuid.h>

#include <string>
#include <vector>

using namespace drogon;
using namespace ikea400;
using namespace ikea400::dao;

Task<std::vector<dto::Vault>> VaultDao::getVaultsByUserId(const uuid& userId) {
  const orm::Result result = co_await m_dbClient->execSqlCoro(
      "SELECT id, name, description, protected_key, salt, EXTRACT(EPOCH FROM "
      "updated_at)::BIGINT AS updated_at, "
      "EXTRACT(EPOCH FROM created_at)::BIGINT AS created_at "
      "FROM vaults WHERE owner_id = $1",
      userId.toString());

  std::vector<dto::Vault> vaults;
  vaults.reserve(result.size());

  for (const auto& row : result) {
    vaults.emplace_back(dto::Vault{
        .id = uuid::fromString(row["id"].as<std::string>()),
        .name = row["name"].as<std::string>(),
        .description = row["description"].as<std::string>(),
        .protected_key = fromBYTEA(row["protected_key"]),
        .salt = fromBYTEA(row["salt"]),
        .updated_at = row["updated_at"].as<std::int64_t>(),
        .created_at = row["created_at"].as<std::int64_t>(),
    });
  }

  co_return vaults;
}

Task<bool> VaultDao::addVault(const uuid& userId,
                              const ikea400::AddVaultRequest& request) {
  try {
    const orm::Result result = co_await m_dbClient->execSqlCoro(
        "INSERT INTO vaults (id, owner_id, name, description, protected_key, "
        "salt) "
        "VALUES ($1, $2, $3, $4, $5, $6)",
        request.id.toString(), userId.toString(), request.name,
        request.description, toBYTEA(request.protected_key.data),
        toBYTEA(request.salt.data));

    co_return result.affectedRows() == 1;
  } catch (const orm::Failure& e) {
    LOG_ERROR << "Failed to add vault: " << e.what();
  }

  co_return false;
}
