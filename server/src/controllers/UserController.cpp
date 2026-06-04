#include "UserController.h"

#include <drogon/RequestStream.h>
#include <drogon/utils/FunctionTraits.h>
#include <drogon/utils/coroutine.h>
#include <dto/response_base.h>
#include <dto/user_dto.h>
#include <dto/vault_dto.h>
#include <trantor/utils/Logger.h>
#include <utils/uuid.h>

#include <exception>
#include <vector>

#include "../dao/UserDao.h"
#include "../dao/VaultDao.h"
#include "../services/SessionService.h"
#include "../utils/ResponseBuilder.h"

using namespace api::v1;
using namespace ikea400;

Task<HttpResponsePtr> api::v1::Users::GetSelf(HttpRequestPtr req) {
  ResponseBuilderPtr builder = ResponseBuilder::get(req);
  try {
    const uuid userId = req->attributes()->get<uuid>("user_id");
    if (userId.isNull()) {
      LOG_ERROR << "No authenticated user found in session";
      co_return builder->unauthorized();
    }

    ResponseMessage<dto::UserProfile> response = {
        .data = co_await dao::UserDao::instance()->getUserProfileById(userId),
        .success = true,
    };

    co_return builder->success(response);
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in GetUserVaults: " << e.what();
  }
  co_return builder->internalServerError();
}

Task<HttpResponsePtr> api::v1::Users::GetUserVaults(HttpRequestPtr req) {
  ResponseBuilderPtr builder = ResponseBuilder::get(req);
  try {
    const uuid userId = req->attributes()->get<uuid>("user_id");
    if (userId.isNull()) {
      LOG_ERROR << "No authenticated user found in session";
      co_return builder->unauthorized();
    }

    ResponseMessage<std::vector<dto::Vault>> response = {
        .data = co_await dao::VaultDao::instance()->getVaultsByUserId(userId),
        .success = true,
    };

    co_return builder->success(response);
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in GetUserVaults: " << e.what();
  }
  co_return builder->internalServerError();
}
