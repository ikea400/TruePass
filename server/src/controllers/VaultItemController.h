#pragma once
#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>
#include <drogon/RequestStream.h>
#include <drogon/utils/FunctionTraits.h>
#include <drogon/utils/coroutine.h>

#include <string>
namespace api::v1 {

class Items : public drogon::HttpController<Items> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(Items::AddItem, "", drogon::Post,
             "AuthFilter");  // POST /api/v1/items
  METHOD_ADD(Items::GetVaultItem, "/{itemId}", drogon::Get,
             "AuthFilter");  // GET /api/v1/items/{itemId}
  METHOD_ADD(Items::EditItem, "/{itemId}", drogon::Put,
             "AuthFilter");  // PUT /api/v1/items/{itemId}
  METHOD_LIST_END
 protected:
  drogon::Task<drogon::HttpResponsePtr> AddItem(drogon::HttpRequestPtr req);
  drogon::Task<drogon::HttpResponsePtr> GetVaultItem(drogon::HttpRequestPtr req,
                                                     std::string itemId);
  drogon::Task<drogon::HttpResponsePtr> EditItem(drogon::HttpRequestPtr req,
                                                 std::string itemId);
};

}  // namespace api::v1