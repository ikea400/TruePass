#pragma once
#include <drogon/HttpAppFramework.h>
#include <drogon/orm/DbClient.h>
#include <trantor/utils/Logger.h>

#include <cstdint>
#include <exception>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ikea400 {
namespace dao {

template <class T>
class BaseDao {
 public:
  BaseDao(drogon::orm::DbClientPtr dbClient)
      : m_dbClient(std::move(dbClient)) {}

  static T* instance() {
    static T* _instance;
    if (_instance) {
      return _instance;
    }

    drogon::orm::DbClientPtr dbClient = drogon::app().getDbClient();
    if (!dbClient) {
      LOG_ERROR << "Database client is not initialized. Please check your "
                   "configuration.";
      throw std::runtime_error{"Database client is not initialized"};
    }
    static T instance = T{dbClient};
    _instance = &instance;
    return _instance;
  }

 protected:
  inline std::vector<char> toBYTEA(std::span<const uint8_t> data) {
    return std::vector<char>(data.begin(), data.end());
  }

  inline std::vector<uint8_t> fromBYTEA(std::span<const char> bytea) {
    return std::vector<uint8_t>(bytea.begin(), bytea.end());
  }

  inline std::vector<uint8_t> fromBYTEA(const drogon::orm::Field& field) {
    return fromBYTEA(field.as<std::vector<char>>());
  }

 protected:
  drogon::orm::DbClientPtr m_dbClient;
};
}  // namespace dao
}  // namespace ikea400