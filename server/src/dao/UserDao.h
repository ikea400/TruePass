#pragma once
#include <drogon/orm/DbClient.h>
#include <drogon/utils/coroutine.h>
#include <utils/uuid.h>
#include <dto/user_dto.h>

#include <cstdint>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "BaseDao.h"

class User;

namespace ikea400 {
namespace dao {

struct AuthCredentials {
  std::string username;
  std::vector<uint8_t> passwordFile;
  uuid id;
};

class UserDao : public BaseDao<UserDao> {
 public:
  UserDao(drogon::orm::DbClientPtr dbClient) : BaseDao(std::move(dbClient)) {}

  drogon::Task<bool> isUsernameOrEmailTaken(const std::string& username,
                                            const std::string& email);

  drogon::Task<bool> createUser(
      const std::string& username, const std::string& email,
      const std::span<const uint8_t>& passwordFile,
      const std::span<const uint8_t> protectedAccountKey);

  drogon::Task<std::optional<AuthCredentials>> getAuthCredentialsByUsername(
      const std::string& username);

  drogon::Task<std::vector<uint8_t>> getProtectedAccountKeyByUserId(
      const uuid& userId);

  drogon::Task<std::optional<dto::UserProfile>> getUserProfileById(const uuid& userId);

 private:
  // Implementation details
};
}  // namespace dao
}  // namespace ikea400