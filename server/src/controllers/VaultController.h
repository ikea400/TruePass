#pragma once
#include <drogon/drogon.h>
namespace api::v1 {

class Vaults : public drogon::HttpController<Vaults> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(Vaults::AddVaults, "", drogon::Post, "AuthFilter");
  METHOD_ADD(Vaults::GetVaultItems, "/{vaultId}/items", drogon::Get, "AuthFilter");
  METHOD_LIST_END
 protected:
  drogon::Task<drogon::HttpResponsePtr> AddVaults(drogon::HttpRequestPtr req);
  drogon::Task<drogon::HttpResponsePtr> GetVaultItems(drogon::HttpRequestPtr req, std::string vaultId);
};

}  // namespace api::v1