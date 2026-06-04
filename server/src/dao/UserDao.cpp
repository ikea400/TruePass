#include "UserDao.h"

#include <drogon/utils/coroutine.h>

#include <cstdint>
#include <string>

using namespace ikea400;
using namespace ikea400::dao;
using namespace drogon;

drogon::Task<bool> UserDao::isUsernameOrEmailTaken(const std::string& username,
                                                   const std::string& email) {
  const auto result = co_await m_dbClient->execSqlCoro(
      "SELECT COUNT(*) AS count FROM users WHERE name = $1 OR email = $2",
      username, email);
  if (result.empty()) {
    co_return false;
  }

  co_return result[0]["count"].as<int>() > 0;
}

drogon::Task<bool> UserDao::createUser(
    const std::string& username, const std::string& email,
    const std::span<const uint8_t>& passwordFile,
    const std::span<const uint8_t> protectedAccountKey) {
  try {
    const auto result = co_await m_dbClient->execSqlCoro(
        "INSERT INTO users (name, email, password_file, protected_account_key) "
        "VALUES ($1, $2, $3, $4)",
        username, email, toBYTEA(passwordFile), toBYTEA(protectedAccountKey));

    co_return result.affectedRows() > 0;
  } catch (const drogon::orm::Failure& e) {
    LOG_ERROR << "Database error while creating user: " << e.what();
  }

  co_return false;
}

drogon::Task<std::optional<AuthCredentials>>
UserDao::getAuthCredentialsByUsername(const std::string& username) {
  const auto result = co_await m_dbClient->execSqlCoro(
      "SELECT id, name, password_file FROM users WHERE name = $1 AND "
      "is_active = TRUE LIMIT 1",
      username);

  if (result.empty()) {
    co_return std::nullopt;
  }

  const auto& row = result[0];

  co_return AuthCredentials{
      .username = row["name"].as<std::string>(),
      .passwordFile = fromBYTEA(row["password_file"]),
      .id = uuid::fromString(row["id"].as<std::string>()),
  };
}

drogon::Task<std::vector<uint8_t>>
ikea400::dao::UserDao::getProtectedAccountKeyByUserId(const uuid& userId) {
  const auto result = co_await m_dbClient->execSqlCoro(
      "SELECT protected_account_key FROM users WHERE id = $1 AND is_active = "
      "TRUE LIMIT 1",
      userId.toString());

  if (result.empty()) {
    co_return {};
  }

  co_return fromBYTEA(result[0]["protected_account_key"]);
}

drogon::Task<std::optional<dto::UserProfile>>
ikea400::dao::UserDao::getUserProfileById(const uuid& userId) {
  const auto result = co_await m_dbClient->execSqlCoro(
      "SELECT name, email FROM users WHERE id = $1 AND is_active = TRUE "
      "LIMIT 1",
      userId.toString());

  if (result.empty()) {
    co_return std::nullopt;
  }

  const auto& row = result[0];
  co_return dto::UserProfile{.userId = userId,
                             .username = row["name"].as<std::string>(),
                             .email = row["email"].as<std::string>()};
}
