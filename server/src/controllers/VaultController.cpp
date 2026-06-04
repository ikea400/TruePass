#include "VaultController.h"

#include <drogon/RequestStream.h>
#include <drogon/utils/FunctionTraits.h>
#include <drogon/utils/coroutine.h>
#include <dto/response_base.h>
#include <dto/vault_dto.h>
#include <dto/vault_item_dto.h>
#include <trantor/utils/Logger.h>
#include <utils/utils.h>
#include <utils/uuid.h>
#include <utils/validation.h>

#include <exception>
#include <string>
#include <vector>

#include "../dao/VaultDao.h"
#include "../dao/VaultItemDao.h"
#include "../services/SessionService.h"
#include "../utils/RequestReader.h"
#include "../utils/ResponseBuilder.h"

using namespace drogon;
using namespace ikea400;

Task<HttpResponsePtr> api::v1::Vaults::AddVaults(HttpRequestPtr req) {
  ResponseBuilderPtr builder = ResponseBuilder::get(req);

  try {
    const uuid userId = req->attributes()->get<uuid>("user_id");
    if (userId.isNull()) {
      LOG_ERROR << "No authenticated user found in session";
      co_return builder->unauthorized();
    }

    LOG_INFO << req->getBody();

    const auto request = RequestReader::read<AddVaultRequest>(*req);
    if (!request) {
      LOG_ERROR << "Failed to read AddVaultRequest from request body";
      co_return builder->badRequest("Invalid request data");
    }

    if (!validation::validateVaultName(request->name).isValid() ||
        !validation::validateVaultDescription(request->description).isValid()) {
      co_return builder->badRequest("Invalid vault name or description");
    }

    if (!ikea400::utils::isEntropySufficient(request->id.bytes()) ||
        !ikea400::utils::isEntropySufficient(request->salt.data) ||
        !ikea400::utils::isEntropySufficient(request->protected_key.data)) {
      co_return builder->badRequest("Insufficient entropy in request data");
    }

    bool success =
        co_await dao::VaultDao::instance()->addVault(userId, *request);
    if (!success) {
      LOG_ERROR << "Failed to add vault for user " << userId.toString();
      co_return builder->badRequest("Invalid vault uuid or name");
    }

    co_return builder->success();
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in AddVaults: " << e.what();
  }

  co_return builder->internalServerError();
}

drogon::Task<drogon::HttpResponsePtr> api::v1::Vaults::GetVaultItems(
    drogon::HttpRequestPtr req, std::string vaultIdStr) {
  ResponseBuilderPtr builder = ResponseBuilder::get(req);
  try {
    const uuid userId = req->attributes()->get<uuid>("user_id");
    if (userId.isNull()) {
      LOG_ERROR << "No authenticated user found in session";
      co_return builder->unauthorized();
    }

    uuid vaultId = uuid::fromString<false>(vaultIdStr);
    if (vaultId.isNull()) {
      LOG_ERROR << "Invalid vault ID format: " << vaultIdStr;
      co_return builder->badRequest("Invalid vault ID");
    }

    ResponseMessage<std::vector<dto::VaultItemSummary>> response{
        .data =
            co_await dao::VaultItemDao::instance()->getUserVaultItemsByVaultId(
                userId, vaultId),
        .success = true,
    };

    co_return builder->success(response);
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in GetVaultItems: " << e.what();
  }

  co_return builder->internalServerError();
}
