#include "VaultItemDao.h"

#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>
#include <drogon/utils/coroutine.h>
#include <dto/vault_item_dto.h>
#include <trantor/utils/Logger.h>
#include <utils/uuid.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

using namespace drogon;
using namespace ikea400;
using namespace ikea400::dao;

Task<std::vector<dto::VaultItemSummary>>
ikea400::dao::VaultItemDao::getUserVaultItemsByVaultId(const uuid& userId,
                                                       const uuid& vaultId) {
  const orm::Result result = co_await m_dbClient->execSqlCoro(
      "SELECT items.id as id, EXTRACT(EPOCH FROM items.created_at)::BIGINT AS "
      "created_at, "
      "items.protected_metadata as protected_metadata "
      "FROM vault_items items "
      "JOIN vaults ON items.vault_id = vaults.id "
      "WHERE vaults.owner_id = $1 AND vaults.id = $2",
      userId.toString(), vaultId.toString());

  std::vector<dto::VaultItemSummary> items;
  items.reserve(result.size());

  for (const auto& row : result) {
    items.emplace_back(dto::VaultItemSummary{
        .id = uuid::fromString(row["id"].as<std::string>()),
        .vaultId = vaultId,
        .protectedMetaData = fromBYTEA(row["protected_metadata"]),
        .createdAt = row["created_at"].as<std::int64_t>(),
    });
  }

  co_return items;
}

Task<bool> ikea400::dao::VaultItemDao::addVaultItem(
    const uuid& userId, const uuid& vaultId,
    const dto::AddVaultItemRequest& vaultItem) {
  try {
    const orm::Result result = co_await m_dbClient->execSqlCoro(
        "INSERT INTO vault_items (id, vault_id, protected_metadata, "
        "protected_payload, name_hash)"
        "SELECT $1, $2, $3, $4, $5 FROM vaults WHERE id = $2 AND owner_id = $6",
        vaultItem.id.toString(), vaultId.toString(),
        toBYTEA(vaultItem.protected_metadata.data),
        toBYTEA(vaultItem.protected_data.data),
        toBYTEA(vaultItem.name_hash.data), userId.toString());
    co_return result.affectedRows() == 1;
  } catch (const orm::Failure& e) {
    LOG_ERROR << "Failed to add vault item: " << e.what();
  }

  co_return false;
}

drogon::Task<bool> ikea400::dao::VaultItemDao::editVaultItem(
    const uuid& userId, const dto::EditVaultItemRequest& vaultItem) {
  try {
    const orm::Result result = co_await m_dbClient->execSqlCoro(
        "UPDATE vault_items SET "
        "protected_metadata = $1, "
        "protected_payload = $2, "
        "name_hash = $3 "
        "WHERE id = $4 AND vault_id = $5 AND EXISTS ("
        "    SELECT 1 FROM vaults WHERE id = $5 AND owner_id = $6"
        ")",
        toBYTEA(vaultItem.protected_metadata.data),
        toBYTEA(vaultItem.protected_data.data),
        toBYTEA(vaultItem.name_hash.data), vaultItem.id.toString(),
        vaultItem.vaultId.toString(), userId.toString());

    co_return result.affectedRows() == 1;
  } catch (const orm::Failure& e) {
    LOG_ERROR << "Failed to edit vault item: " << e.what();
  }
  co_return false;
}

drogon::Task<std::optional<dto::VaultItem>>
ikea400::dao::VaultItemDao::getVaultItemById(const uuid& userId,
                                             const uuid& itemId) {
  const orm::Result result = co_await m_dbClient->execSqlCoro(
      "SELECT items.id as id, items.vault_id as vault_id, "
      "items.protected_metadata as protected_metadata, "
      "items.protected_payload as protected_payload, "
      "EXTRACT(EPOCH FROM items.created_at)::BIGINT AS created_at, "
      "EXTRACT(EPOCH FROM items.updated_at)::BIGINT AS updated_at "
      "FROM vault_items items "
      "JOIN vaults ON items.vault_id = vaults.id "
      "WHERE vaults.owner_id = $1 AND items.id = $2",
      userId.toString(), itemId.toString());

  if (result.empty()) co_return std::nullopt;

  const auto& row = result.front();

  co_return dto::VaultItem{
      .id = uuid::fromString(row["id"].as<std::string>()),
      .vaultId = uuid::fromString(row["vault_id"].as<std::string>()),
      .protected_metadata = fromBYTEA(row["protected_metadata"]),
      .protected_data = fromBYTEA(row["protected_payload"]),
      .created_at = row["created_at"].as<std::int64_t>(),
      .updated_at = row["updated_at"].as<std::int64_t>(),
  };
}
