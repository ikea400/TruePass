#include "VaultItemController.h"

#include <drogon/RequestStream.h>
#include <drogon/utils/FunctionTraits.h>
#include <drogon/utils/coroutine.h>
#include <dto/response_base.h>
#include <dto/vault_item_dto.h>
#include <trantor/utils/Logger.h>
#include <utils/utils.h>
#include <utils/uuid.h>

#include <exception>
#include <optional>
#include <string>
#include <utility>

#include "../dao/VaultItemDao.h"
#include "../services/SessionService.h"
#include "../utils/RequestReader.h"
#include "../utils/ResponseBuilder.h"

using namespace drogon;
using namespace ikea400;

Task<HttpResponsePtr> api::v1::Items::AddItem(HttpRequestPtr req) {
  ResponseBuilderPtr builder = ikea400::ResponseBuilder::get(req);

  try {
    const uuid userId = req->attributes()->get<uuid>("user_id");
    if (userId.isNull()) {
      LOG_ERROR << "No authenticated user found in session";
      co_return builder->unauthorized();
    }

    LOG_INFO << req->getBody();

    const auto requestOpt = RequestReader::read<dto::AddVaultItemRequest>(*req);
    if (!requestOpt) {
      LOG_INFO << "Failed to read request data";
      co_return builder->badRequest("Invalid request data");
    }

    const auto& request = *requestOpt;
    if (!ikea400::utils::isEntropySufficient(request.name_hash.data) ||
        !ikea400::utils::isEntropySufficient(request.protected_metadata.data) ||
        !ikea400::utils::isEntropySufficient(request.protected_data.data) ||
        !ikea400::utils::isEntropySufficient(request.id.bytes())) {
      LOG_INFO << "Request data does not have sufficient entropy";
      co_return builder->badRequest(
          "Request data does not have sufficient entropy");
    }

    bool success = co_await dao::VaultItemDao::instance()->addVaultItem(
        userId, request.vaultId, request);
    if (!success) {
      // Failure is indicated by a not found vault owened by the user, a
      // duplicate item ID or a duplicate item name for the user.
      co_return builder->badRequest("Failed to add vault item.");
    }

    co_return builder->success();
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in AddItem: " << e.what();
  }

  co_return builder->internalServerError();
}

Task<HttpResponsePtr> api::v1::Items::GetVaultItem(HttpRequestPtr req,
                                                   std::string itemId) {
  ResponseBuilderPtr builder = ikea400::ResponseBuilder::get(req);

  try {
    const uuid userId = req->attributes()->get<uuid>("user_id");
    if (userId.isNull()) {
      LOG_ERROR << "No authenticated user found in session";
      co_return builder->unauthorized();
    }

    uuid itemUuid = uuid::fromString<false>(itemId);
    if (itemUuid.isNull()) {
      LOG_INFO << "Invalid item ID format: " << itemId;
      co_return builder->badRequest("Invalid item ID format");
    }

    std::optional<dto::VaultItem> vaultItemOpt =
        co_await dao::VaultItemDao::instance()->getVaultItemById(userId,
                                                                 itemUuid);
    if (!vaultItemOpt) {
      LOG_INFO << "Vault item not found for user: " << userId.toString()
               << ", item ID: " << itemId;
      co_return builder->notFound("Vault item not found");
    }

    ResponseMessage<dto::VaultItem> response{
        .data = std::move(vaultItemOpt).value(),
        .success = true,
    };

    co_return builder->success(response);
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in GetVaultItem: " << e.what();
  }

  co_return builder->internalServerError();
}

drogon::Task<drogon::HttpResponsePtr> api::v1::Items::EditItem(
    drogon::HttpRequestPtr req, std::string itemId) {
  ResponseBuilderPtr builder = ikea400::ResponseBuilder::get(req);

  try {
    const uuid userId = req->attributes()->get<uuid>("user_id");
    if (userId.isNull()) {
      LOG_ERROR << "No authenticated user found in session";
      co_return builder->unauthorized();
    }

    uuid itemUuid = uuid::fromString<false>(itemId);
    if (itemUuid.isNull()) {
      LOG_INFO << "Invalid item ID format: " << itemId;
      co_return builder->badRequest("Invalid item ID format");
    }

    const auto requestOpt =
        RequestReader::read<dto::EditVaultItemRequest>(*req);
    if (!requestOpt) {
      LOG_INFO << "Failed to read request data";
      co_return builder->badRequest("Invalid request data");
    }

    dao::VaultItemDao* vaultItemDao = dao::VaultItemDao::instance();
    bool success = co_await vaultItemDao->editVaultItem(userId, *requestOpt);
    if (!success) {
      co_return builder->badRequest("Failed to edit vault item.");
    }

    co_return builder->success();
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in EditItem: " << e.what();
  }
  co_return builder->internalServerError();
}
