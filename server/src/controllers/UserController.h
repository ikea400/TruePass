#pragma once
#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>
#include <drogon/RequestStream.h>
#include <drogon/utils/FunctionTraits.h>
#include <drogon/utils/coroutine.h>

namespace api::v1 {
using namespace drogon;

class Users : public HttpController<Users> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(Users::GetSelf, "/me", Get, "AuthFilter"); // path is /api/v1/users/me
  METHOD_ADD(Users::GetUserVaults, "/me/vaults", Get, "AuthFilter"); // path is /api/v1/users/me/vaults
  METHOD_LIST_END
 protected:
  Task<HttpResponsePtr> GetSelf(HttpRequestPtr req);
  Task<HttpResponsePtr> GetUserVaults(HttpRequestPtr req);
};

}  // namespace api::v1