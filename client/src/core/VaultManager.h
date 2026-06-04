#pragma once

#include <dto/response_base.h>
#include <dto/vault_dto.h>
#include <dto/vault_item_dto.h>
#include <qobject.h>
#include <qstring.h>
#include <qtmetamacros.h>
#include <utils/uuid.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../model/VaultItemModel.h"
#include "../network/RequestError.h"
#include "Vault.h"
#include "VaultItem.h"

class ServerSession;
class MasterKeyManager;

class VaultManager : public QObject {
  Q_OBJECT

 public:
  explicit VaultManager(ServerSession* session);

  [[nodiscard]] const std::unordered_map<ikea400::uuid, Vault>& getVaults()
      const noexcept;
  [[nodiscard]] const std::unordered_map<
      ikea400::uuid, std::unordered_map<ikea400::uuid, VaultItem>>&
  getVaultItems() const noexcept;
  [[nodiscard]] std::optional<VaultItem> getVaultItem(
      ikea400::uuid vaultId, ikea400::uuid itemId) const noexcept;

 signals:
  void vaultListUpdated();
  void vaultListUpdateFailed(const QString& errorMessage);
  void addVaultFailed(const QString& errorMessage);
  void vaultAdded(ikea400::uuid vaultId);
  void vaultOpened(ikea400::uuid vaultId);
  void vaultOpenFailed(ikea400::uuid vaultId, const QString& errorMessage);
  void addItemFailed(const QString& errorMessage);
  void itemAdded(ikea400::uuid vaultId, ikea400::uuid itemId);
  void itemOpenFailed(ikea400::uuid vaultId, ikea400::uuid itemId,
                      const QString& errorMessage);
  void itemOpened(ikea400::uuid vaultId, const VaultItemModel& item);
  void editItemFailed(const QString& errorMessage);
  void itemEdited(ikea400::uuid vaultId, const VaultItem& item,
                  const VaultItemModel& itemModel);

 public slots:
  void updateVaultList();
  void openVault(ikea400::uuid vaultId);
  void closeVault(ikea400::uuid vaultId);
  void addVault(const QString& name, const QString& description);
  void addItem(const VaultItemModel& item, ikea400::uuid vaultId);
  void openItem(ikea400::uuid vaultId, ikea400::uuid itemId);
  void editItem(const VaultItemModel& item, ikea400::uuid vaultId);

 private:
  void onFetchVaultListResult(
      RequestError error,
      const ikea400::ResponseMessage<std::vector<ikea400::dto::Vault>>&
          response);
  void onAddVaultComputed(Vault&& vault);
  void onAddVaultResult(RequestError error,
                        const ikea400::ResponseMessage<void>& response,
                        Vault&& vault);
  void onOpenVaultResult(
      ikea400::uuid vaultId, RequestError error,
      const ikea400::ResponseMessage<
          std::vector<ikea400::dto::VaultItemSummary>>& response);
  void onAddItemResult(RequestError error,
                       const ikea400::ResponseMessage<void>& response,
                       ikea400::uuid vaultId, VaultItem&& vaultItem);
  void onOpenItemResult(
      ikea400::uuid vaultId, ikea400::uuid itemId, RequestError error,
      const ikea400::ResponseMessage<ikea400::dto::VaultItem>& response);
  void onEditItemResult(RequestError error,
                        const ikea400::ResponseMessage<void>& response,
                        ikea400::uuid vaultId, VaultItem&& vaultItem);

 private:
  bool checkIsWorking(const QString& operation,
                      bool stopWorking = true) noexcept;

  bool validateVaultName(const QString& name) const noexcept;
  bool validateVaultDescription(const QString& description) const noexcept;
  bool validateVaultItemInput(const QString& name,
                              const QString& note) const noexcept;

  Vault* findVault(ikea400::uuid vaultId) noexcept;
  const Vault* findVault(ikea400::uuid vaultId) const noexcept;
  VaultItem* findVaultItem(ikea400::uuid vaultId,
                           ikea400::uuid itemId) noexcept;
  const VaultItem* findVaultItem(ikea400::uuid vaultId,
                                 ikea400::uuid itemId) const noexcept;

  bool vaultNameExists(ikea400::uuid vaultId,
                       const QString& name) const noexcept;
  bool itemNameExists(ikea400::uuid vaultId, ikea400::uuid excludeItemId,
                      const QString& name) const noexcept;

  void handleRequestError(const QString& errorMessage,
                          std::function<void(const QString&)> errorHandler,
                          bool stopWorking = true) noexcept;

  void onUpdateVaultListError(const QString& errorMessage,
                              bool stopWorking = true) noexcept;
  void onUpdateVaultListSuccess() noexcept;
  void onAddVaultError(const QString& errorMessage,
                       bool stopWorking = true) noexcept;
  void onAddVaultSuccess(ikea400::uuid vaultId) noexcept;
  void onOpenVaultError(ikea400::uuid vaultId, const QString& errorMessage,
                        bool stopWorking = true) noexcept;
  void onOpenVaultSuccess(ikea400::uuid vaultId) noexcept;
  void onAddItemError(const QString& errorMessage,
                      bool stopWorking = true) noexcept;
  void onAddItemSuccess(ikea400::uuid vaultId, ikea400::uuid itemId) noexcept;
  void onOpenItemError(ikea400::uuid vaultId, ikea400::uuid itemId,
                       const QString& errorMessage,
                       bool stopWorking = true) noexcept;
  void onOpenItemSuccess(ikea400::uuid vaultId,
                         const VaultItemModel& item) noexcept;
  void onOpenItemSuccess(ikea400::uuid vaultId, const VaultItem& item) noexcept;
  void onEditItemError(const QString& errorMessage,
                       bool stopWorking = true) noexcept;
  void onEditItemSuccess(ikea400::uuid vaultId, const VaultItem& item,
                         const VaultItemModel& itemModel) noexcept;
  void onEditItemSuccess(ikea400::uuid vaultId, const VaultItem& item) noexcept;

  using VaultMap = std::unordered_map<ikea400::uuid, Vault>;
  using VaultItemMap =
      std::unordered_map<ikea400::uuid,
                         std::unordered_map<ikea400::uuid, VaultItem>>;

  VaultMap m_vaults;
  VaultItemMap m_vaultItems;
  ServerSession* m_session;
  MasterKeyManager* m_masterKeyManager;
  bool m_isWorking = false;
};