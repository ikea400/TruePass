#include "VaultManager.h"

#include <dto/response_base.h>
#include <dto/vault_dto.h>
#include <dto/vault_item_dto.h>
#include <qfuturewatcher.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qstring.h>
#include <qtconcurrentrun.h>
#include <qtmetamacros.h>
#include <utils/uuid.h>
#include <utils/validation.h>

#include <algorithm>
#include <exception>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../model/VaultItemModel.h"
#include "../network/RequestError.h"
#include "../network/ServerSession.h"
#include "QDefaultException.h"
#include "Vault.h"
#include "VaultItem.h"

using namespace ikea400;

namespace {
constexpr std::string_view kApiVaultsEndpoint = "/api/v1/users/me/vaults";
constexpr std::string_view kApiVaultItemsEndpoint = "/api/v1/vaults/{}/items";
constexpr std::string_view kApiItemsEndpoint = "/api/v1/items";
constexpr std::string_view kApiItemEndpoint = "/api/v1/items/{}";

constexpr std::string_view kErrorOperationInProgress =
    "Another operation is already in progress. Please wait.";
constexpr std::string_view kErrorInvalidVaultId = "Invalid vault ID.";
constexpr std::string_view kErrorVaultNotFound = "Vault not found.";
constexpr std::string_view kErrorInvalidInput =
    "Invalid vault name or description. Please ensure they meet the required "
    "criteria.";
constexpr std::string_view kErrorItemNameExists =
    "An item with the same name already exists in this vault.";
constexpr std::string_view kErrorVaultNameExists =
    "A vault with the same name already exists.";
constexpr std::string_view kErrorFailedDecryptKey =
    "Failed to decrypt vault key.";
constexpr std::string_view kErrorNoData = "No data in response.";
constexpr std::string_view kErrorItemNotFound = "Item not found in vault.";
constexpr std::string_view kErrorDeletedItem = "Cannot edit a deleted item.";
}  // namespace

VaultManager::VaultManager(ServerSession* session)
    : m_session(session),
      m_masterKeyManager(session->authService()->getMasterKeyManager()) {}

const std::unordered_map<uuid, Vault>& VaultManager::getVaults()
    const noexcept {
  return m_vaults;
}

const std::unordered_map<uuid, std::unordered_map<uuid, VaultItem>>&
VaultManager::getVaultItems() const noexcept {
  return m_vaultItems;
}

std::optional<VaultItem> VaultManager::getVaultItem(
    uuid vaultId, uuid itemId) const noexcept {
  auto vaultIt = m_vaultItems.find(vaultId);
  if (vaultIt == m_vaultItems.end()) {
    qWarning() << "Vault not found for getting item:"
               << vaultId.toString().c_str();
    return std::nullopt;
  }

  auto itemIt = vaultIt->second.find(itemId);
  if (itemIt == vaultIt->second.end()) {
    qWarning() << "Item not found in vault:" << itemId.toString().c_str();
    return std::nullopt;
  }

  return itemIt->second;
}

bool VaultManager::checkIsWorking(const QString& operation,
                                  bool stopWorking) noexcept {
  if (m_isWorking) {
    if (stopWorking) m_isWorking = false;
    return true;
  }
  return false;
}

bool VaultManager::validateVaultName(const QString& name) const noexcept {
  return validation::validateVaultName(name.toStdString()).isValid();
}

bool VaultManager::validateVaultDescription(
    const QString& description) const noexcept {
  return validation::validateVaultDescription(description.toStdString())
      .isValid();
}

bool VaultManager::validateVaultItemInput(const QString& name,
                                          const QString& note) const noexcept {
  return validation::validateVaultItemName(name.toStdString()).isValid() &&
         validation::validateVaultItemNote(note.toStdString()).isValid();
}

Vault* VaultManager::findVault(uuid vaultId) noexcept {
  auto it = m_vaults.find(vaultId);
  return it != m_vaults.end() ? &it->second : nullptr;
}

const Vault* VaultManager::findVault(uuid vaultId) const noexcept {
  auto it = m_vaults.find(vaultId);
  return it != m_vaults.end() ? &it->second : nullptr;
}

VaultItem* VaultManager::findVaultItem(uuid vaultId, uuid itemId) noexcept {
  auto vaultIt = m_vaultItems.find(vaultId);
  if (vaultIt == m_vaultItems.end()) return nullptr;

  auto itemIt = vaultIt->second.find(itemId);
  return itemIt != vaultIt->second.end() ? &itemIt->second : nullptr;
}

const VaultItem* VaultManager::findVaultItem(uuid vaultId,
                                             uuid itemId) const noexcept {
  auto vaultIt = m_vaultItems.find(vaultId);
  if (vaultIt == m_vaultItems.end()) return nullptr;

  auto itemIt = vaultIt->second.find(itemId);
  return itemIt != vaultIt->second.end() ? &itemIt->second : nullptr;
}

bool VaultManager::vaultNameExists(uuid vaultId,
                                   const QString& name) const noexcept {
  return std::find_if(
             m_vaults.begin(), m_vaults.end(), [&name](const auto& pair) {
               return name.compare(
                          QString::fromStdString(pair.second.getName()),
                          Qt::CaseInsensitive) == 0;
             }) != m_vaults.end();
}

bool VaultManager::itemNameExists(uuid vaultId, uuid excludeItemId,
                                  const QString& name) const noexcept {
  auto vaultIt = m_vaultItems.find(vaultId);
  if (vaultIt == m_vaultItems.end()) return false;

  return std::find_if(vaultIt->second.begin(), vaultIt->second.end(),
                      [&name, &excludeItemId](const auto& pair) {
                        return pair.first != excludeItemId &&
                               name.compare(QString::fromStdString(
                                                pair.second.getName()),
                                            Qt::CaseInsensitive) == 0;
                      }) != vaultIt->second.end();
}

void VaultManager::updateVaultList() {
  if (checkIsWorking("updateVaultList")) {
    onUpdateVaultListError(
        QString::fromStdString(std::string(kErrorOperationInProgress)));
    return;
  }

  m_isWorking = true;
  m_session->getRequest<std::vector<dto::Vault>>(
      QString::fromStdString(std::string(kApiVaultsEndpoint)),
      std::bind(&VaultManager::onFetchVaultListResult, this,
                std::placeholders::_1, std::placeholders::_2));
}

void VaultManager::openVault(uuid vaultId) {
  if (checkIsWorking("openVault")) {
    onOpenVaultError(
        vaultId, QString::fromStdString(std::string(kErrorOperationInProgress)),
        false);
    return;
  }

  if (vaultId.isNull()) {
    onOpenVaultError(vaultId,
                     QString::fromStdString(std::string(kErrorInvalidVaultId)));
    return;
  }

  Vault* vault = findVault(vaultId);
  if (!vault) {
    onOpenVaultError(vaultId,
                     QString::fromStdString(std::string(kErrorVaultNotFound)));
    return;
  }

  if (!vault->decryptVaultKey(m_masterKeyManager)) {
    onOpenVaultError(
        vaultId, QString::fromStdString(std::string(kErrorFailedDecryptKey)));
    return;
  }

  m_isWorking = true;
  m_session->getRequest<std::vector<dto::VaultItemSummary>>(
      QString("/api/v1/vaults/%1/items").arg(vaultId.toString().c_str()),
      [this, vaultId](
          RequestError error,
          const ResponseMessage<std::vector<dto::VaultItemSummary>>& response) {
        onOpenVaultResult(vaultId, error, response);
      });
}

void VaultManager::closeVault(uuid vaultId) {}

void VaultManager::addVault(const QString& name, const QString& description) {
  if (checkIsWorking("addVault")) {
    onAddVaultError(
        QString::fromStdString(std::string(kErrorOperationInProgress)), false);
    return;
  }

  if (!validateVaultName(name) || !validateVaultDescription(description)) {
    onAddVaultError(QString::fromStdString(std::string(kErrorInvalidInput)));
    return;
  }

  if (vaultNameExists(uuid{}, name)) {
    onAddVaultError(QString::fromStdString(std::string(kErrorVaultNameExists)));
    return;
  }

  m_isWorking = true;
  auto future = QtConcurrent::run([this, name, description]() {
    try {
      return Vault(m_masterKeyManager, name.toStdString(),
                   description.toStdString());
    } catch (const std::exception& e) {
      throw QDefaultException(QString::fromStdString(e.what()));
    }
  });

  auto watcher = new QFutureWatcher<Vault>(this);
  connect(watcher, &QFutureWatcher<Vault>::finished, this, [this, watcher]() {
    try {
      Vault newVault = watcher->result();
      onAddVaultComputed(std::move(newVault));
    } catch (const std::exception& e) {
      qWarning() << "Add Vault Error:" << e.what();
      onAddVaultError("Add Vault Error:" + QString::fromStdString(e.what()));
    }
    watcher->deleteLater();
  });

  watcher->setFuture(future);
}

void VaultManager::addItem(const VaultItemModel& item, uuid vaultId) {
  if (checkIsWorking("addItem")) {
    onAddItemError(
        QString::fromStdString(std::string(kErrorOperationInProgress)), false);
    return;
  }

  if (!validateVaultItemInput(item.getName(), item.getNote())) {
    onAddItemError(QString::fromStdString(std::string(kErrorInvalidInput)));
    return;
  }

  if (itemNameExists(vaultId, uuid{}, item.getName())) {
    onAddItemError(QString::fromStdString(std::string(kErrorItemNameExists)));
    return;
  }

  Vault* vault = findVault(vaultId);
  if (!vault) {
    qWarning() << "Vault not found for adding item:"
               << vaultId.toString().c_str();
    onAddItemError(QString::fromStdString(std::string(kErrorVaultNotFound)));
    return;
  }

  VaultItem newItem;
  try {
    newItem = VaultItem::createFromModel(item, *vault);
  } catch (const std::exception& e) {
    qWarning() << "Failed to create vault item from model:" << e.what();
    onAddItemError("Failed to create item: " +
                   QString::fromStdString(e.what()));
    return;
  }

  const auto nameHashOpt =
      vault->getItemNameHash(item.getName().toLower().toStdString());
  if (!nameHashOpt) {
    qWarning() << "Failed to compute name hash for item:" << item.getName();
    onAddItemError("Failed to compute name hash for item.");
    return;
  }

  auto newItemDto = dto::AddVaultItemRequest{
      .id = newItem.getId(),
      .vaultId = vaultId,
      .protected_metadata = newItem.getEncryptedMetadata(),
      .protected_data = newItem.getEncryptedData(),
      .name_hash = *nameHashOpt};

  m_isWorking = true;
  m_session->postRequest<void>(
      QString::fromStdString(std::string(kApiItemsEndpoint)), newItemDto,
      [this, vaultId, newItem = std::move(newItem)](
          RequestError error, const ResponseMessage<void>& response) mutable {
        onAddItemResult(error, response, vaultId, std::move(newItem));
      });
}

void VaultManager::openItem(uuid vaultId, uuid itemId) {
  if (checkIsWorking("openItem")) {
    onOpenItemError(
        vaultId, itemId,
        QString::fromStdString(std::string(kErrorOperationInProgress)), false);
    return;
  }

  Vault* vault = findVault(vaultId);
  if (!vault) {
    onOpenItemError(vaultId, itemId,
                    QString::fromStdString(std::string(kErrorVaultNotFound)));
    return;
  }

  VaultItem* item = findVaultItem(vaultId, itemId);
  if (!item) {
    onOpenItemError(vaultId, itemId,
                    QString::fromStdString(std::string(kErrorItemNotFound)));
    return;
  }

  if (item->hasData()) {
    onOpenItemSuccess(vaultId, *item);
    return;
  }

  m_isWorking = true;
  m_session->getRequest<dto::VaultItem>(
      QString("/api/v1/items/%1").arg(itemId.toString().c_str()),
      [this, vaultId, itemId](RequestError error,
                              const ResponseMessage<dto::VaultItem>& response) {
        onOpenItemResult(vaultId, itemId, error, response);
      });
}

void VaultManager::editItem(const VaultItemModel& item, uuid vaultId) {
  if (checkIsWorking("editItem")) {
    onEditItemError(
        QString::fromStdString(std::string(kErrorOperationInProgress)), false);
    return;
  }

  if (!validateVaultItemInput(item.getName(), item.getNote())) {
    onEditItemError(QString::fromStdString(std::string(kErrorInvalidInput)));
    return;
  }

  Vault* vault = findVault(vaultId);
  if (!vault) {
    qWarning() << "Vault not found for editing item:"
               << vaultId.toString().c_str();
    onEditItemError(QString::fromStdString(std::string(kErrorVaultNotFound)));
    return;
  }

  uuid itemId = uuid::fromString<false>(item.getId().toStdString());
  if (itemId.isNull()) {
    onEditItemError(QString::fromStdString(std::string(kErrorInvalidVaultId)));
    return;
  }

  VaultItem* existingItem = findVaultItem(vaultId, itemId);
  if (!existingItem) {
    qWarning() << "Item not found in vault for editing item:"
               << itemId.toString().c_str();
    onEditItemError(QString::fromStdString(std::string(kErrorItemNotFound)));
    return;
  }

  if (existingItem->isDeleted()) {
    onEditItemError(QString::fromStdString(std::string(kErrorDeletedItem)));
    return;
  }

  if (item.getName().compare(QString::fromStdString(existingItem->getName()),
                             Qt::CaseInsensitive) != 0) {
    if (itemNameExists(vaultId, itemId, item.getName())) {
      onEditItemError(
          QString::fromStdString(std::string(kErrorItemNameExists)));
      return;
    }
  }

  VaultItem newItem;
  try {
    newItem = VaultItem::createFromModel(item, *vault);
  } catch (const std::exception& e) {
    qWarning() << "Failed to create vault item from model for editing:"
               << e.what();
    onEditItemError("Failed to create item for editing: " +
                    QString::fromStdString(e.what()));
    return;
  }

  const auto nameHashOpt =
      vault->getItemNameHash(item.getName().toLower().toStdString());
  if (!nameHashOpt) {
    qWarning() << "Failed to compute name hash for item:" << item.getName();
    onEditItemError("Failed to compute name hash for item.");
    return;
  }

  auto newItemDto = dto::EditVaultItemRequest{
      .id = newItem.getId(),
      .vaultId = vaultId,
      .protected_metadata = newItem.getEncryptedMetadata(),
      .protected_data = newItem.getEncryptedData(),
      .name_hash = *nameHashOpt};

  m_session->putRequest<void>(
      QString("/api/v1/items/%1").arg(itemId.toString().c_str()), newItemDto,
      [this, vaultId, itemId, newItem](
          RequestError error, const ResponseMessage<void>& response) mutable {
        onEditItemResult(error, response, vaultId, std::move(newItem));
      });
}

void VaultManager::toggleFavorite(uuid vaultId, uuid itemId, bool isFavorite) {
  if (checkIsWorking("toggleFavorite")) {
    onEditItemError(
        QString::fromStdString(std::string(kErrorOperationInProgress)), false);
    return;
  }

  Vault* vault = findVault(vaultId);
  if (!vault) {
    qWarning() << "Vault not found for toggling favorite:"
               << vaultId.toString().c_str();
    onEditItemError(QString::fromStdString(std::string(kErrorVaultNotFound)));
    return;
  }

  VaultItem* existingItem = findVaultItem(vaultId, itemId);
  if (!existingItem) {
    qWarning() << "Item not found in vault for toggling favorite:"
               << itemId.toString().c_str();
    onEditItemError(QString::fromStdString(std::string(kErrorItemNotFound)));
    return;
  }

  if (existingItem->isDeleted()) {
    onEditItemError(QString::fromStdString(std::string(kErrorDeletedItem)));
    return;
  }

  // Update the favorite state in the existing item
  existingItem->setFavorite(isFavorite);

  // Create a new item with the updated favorite state
  VaultItem updatedItem;
  try {
    VaultItemModel model = existingItem->toModel(*vault);
    updatedItem = VaultItem::createFromModel(model, *vault);
  } catch (const std::exception& e) {
    qWarning() << "Failed to create vault item from model for favorite toggle:"
               << e.what();
    onEditItemError("Failed to update favorite: " +
                    QString::fromStdString(e.what()));
    return;
  }

  const auto nameHashOpt = vault->getItemNameHash(
      QString::fromStdString(existingItem->getName()).toLower().toStdString());
  if (!nameHashOpt) {
    qWarning() << "Failed to compute name hash for item:"
               << existingItem->getName().c_str();
    onEditItemError("Failed to compute name hash for item.");
    return;
  }

  auto updatedItemDto = dto::EditVaultItemRequest{
      .id = updatedItem.getId(),
      .vaultId = vaultId,
      .protected_metadata = updatedItem.getEncryptedMetadata(),
      .protected_data = updatedItem.getEncryptedData(),
      .name_hash = *nameHashOpt};

  m_session->putRequest<void>(
      QString("/api/v1/items/%1").arg(itemId.toString().c_str()),
      updatedItemDto,
      [this, vaultId, itemId, isFavorite](
          RequestError error, const ResponseMessage<void>& response) {
        if (error != RequestError::None) {
          onEditItemError(QString::fromStdString(
              response.error.value_or(requestErrorToString(error))));
          return;
        }

        onFavoriteToggled(vaultId, itemId, isFavorite);
      });
}

void VaultManager::onFavoriteToggled(uuid vaultId, uuid itemId,
                                     bool isFavorite) noexcept {
  emit favoriteToggled(vaultId, itemId, isFavorite);
}

void VaultManager::onFetchVaultListResult(
    RequestError error,
    const ResponseMessage<std::vector<dto::Vault>>& response) {
  if (error != RequestError::None) {
    onUpdateVaultListError(QString::fromStdString(
        response.error.value_or(requestErrorToString(error))));
    return;
  }

  if (!response.data) {
    onUpdateVaultListError(QString::fromStdString(std::string(kErrorNoData)));
    return;
  }

  m_vaults.clear();
  for (const auto& vaultDto : *response.data) {
    m_vaults.emplace(vaultDto.id, Vault(vaultDto));
  }

  onUpdateVaultListSuccess();
}

void VaultManager::onAddVaultComputed(Vault&& vault) {
  AddVaultRequest request{
      .id = vault.getId(),
      .name = vault.getName(),
      .description = vault.getDescription(),
      .protected_key = vault.getProtectedKeyBlob(),
      .salt = vault.getSalt(),
  };

  m_session->postRequest<void>(
      QString::fromStdString(std::string(kApiVaultsEndpoint)), request,
      [this, vault = std::move(vault)](
          RequestError error, const ResponseMessage<void>& response) mutable {
        onAddVaultResult(error, response, std::move(vault));
      });
}

void VaultManager::onAddVaultResult(RequestError error,
                                    const ResponseMessage<void>& response,
                                    Vault&& vault) {
  if (error != RequestError::None) {
    onAddVaultError(QString::fromStdString(
        response.error.value_or(requestErrorToString(error))));
    return;
  }

  const uuid vaultId = vault.getId();
  m_vaults.emplace(vault.getId(), std::move(vault));
  onAddVaultSuccess(vaultId);
}

void VaultManager::onOpenVaultResult(
    uuid vaultId, RequestError error,
    const ResponseMessage<std::vector<dto::VaultItemSummary>>& response) {
  if (error != RequestError::None) {
    onOpenVaultError(vaultId, QString::fromStdString(response.error.value_or(
                                  requestErrorToString(error))));
    return;
  }

  if (!response.data) {
    onOpenVaultError(vaultId,
                     QString::fromStdString(std::string(kErrorNoData)));
    return;
  }

  const Vault* vault = findVault(vaultId);
  if (!vault) {
    onOpenVaultError(vaultId,
                     QString::fromStdString(std::string(kErrorVaultNotFound)));
    return;
  }

  m_vaultItems[vaultId].clear();

  try {
    for (const auto& itemSummary : *response.data) {
      m_vaultItems[vaultId].emplace(
          itemSummary.id, VaultItem::loadFromSummary(itemSummary, *vault));
    }
  } catch (const std::exception& e) {
    qWarning() << "Failed to load vault items for vault ID"
               << vaultId.toString().c_str() << ":" << e.what();
    onOpenVaultError(vaultId, "Failed to load vault items: " +
                                  QString::fromStdString(e.what()));
    return;
  }

  onOpenVaultSuccess(vaultId);
}

void VaultManager::onAddItemResult(RequestError error,
                                   const ResponseMessage<void>& response,
                                   uuid vaultId, VaultItem&& vaultItem) {
  if (error != RequestError::None) {
    const QString errorMsg = QString::fromStdString(
        response.error.value_or(requestErrorToString(error)));
    qWarning() << "Failed to add item to vault ID" << vaultId.toString().c_str()
               << ":" << errorMsg;
    onAddItemError("Failed to add item: " + errorMsg);
    return;
  }

  const uuid itemId = vaultItem.getId();
  m_vaultItems[vaultId].emplace(itemId, std::move(vaultItem));
  onAddItemSuccess(vaultId, itemId);
}

void VaultManager::onOpenItemResult(
    uuid vaultId, uuid itemId, RequestError error,
    const ResponseMessage<dto::VaultItem>& response) {
  if (error != RequestError::None) {
    onOpenItemError(vaultId, itemId,
                    QString::fromStdString(
                        response.error.value_or(requestErrorToString(error))));
    return;
  }

  if (!response.data) {
    onOpenItemError(vaultId, itemId,
                    QString::fromStdString(std::string(kErrorNoData)));
    return;
  }

  const Vault* vault = findVault(vaultId);
  if (!vault) {
    onOpenItemError(vaultId, itemId,
                    QString::fromStdString(std::string(kErrorVaultNotFound)));
    return;
  }

  VaultItem* item = findVaultItem(vaultId, itemId);
  if (!item) {
    onOpenItemError(vaultId, itemId,
                    QString::fromStdString(std::string(kErrorItemNotFound)));
    return;
  }

  if (!item->hasData()) {
    item->setEncryptedData(response.data->protected_data.data);
  }

  onOpenItemSuccess(vaultId, *item);
}

void VaultManager::onEditItemResult(RequestError error,
                                    const ResponseMessage<void>& response,
                                    uuid vaultId, VaultItem&& vaultItem) {
  if (error != RequestError::None) {
    const QString errorMsg = QString::fromStdString(
        response.error.value_or(requestErrorToString(error)));
    qWarning() << "Failed to edit item in vault ID"
               << vaultId.toString().c_str() << ":" << errorMsg;
    onEditItemError("Failed to edit item: " + errorMsg);
    return;
  }

  const Vault* vault = findVault(vaultId);
  if (!vault) {
    onEditItemError("Failed to edit item: Vault not found");
    return;
  }

  const uuid itemId = vaultItem.getId();
  const VaultItem* item = findVaultItem(vaultId, itemId);
  if (!item) {
    onEditItemError("Failed to edit item: Item not found in vault");
    return;
  }

  auto& vaultMap = m_vaultItems[vaultId];
  auto [it, inserted] = vaultMap.insert_or_assign(itemId, std::move(vaultItem));

  onEditItemSuccess(vaultId, it->second);
}

void VaultManager::onUpdateVaultListError(const QString& errorMessage,
                                          bool stopWorking) noexcept {
  qDebug("Failed to update vault list: %s", qPrintable(errorMessage));
  emit vaultListUpdateFailed(errorMessage);
  if (stopWorking) m_isWorking = false;
}

void VaultManager::onUpdateVaultListSuccess() noexcept {
  qDebug("Vault list updated successfully.");
  emit vaultListUpdated();
  m_isWorking = false;
}

void VaultManager::onAddVaultError(const QString& errorMessage,
                                   bool stopWorking) noexcept {
  qDebug("Failed to add vault: %s", qPrintable(errorMessage));
  emit addVaultFailed(errorMessage);
  if (stopWorking) m_isWorking = false;
}

void VaultManager::onAddVaultSuccess(uuid vaultId) noexcept {
  qDebug("Vault added successfully with ID: %s", vaultId.toString().c_str());
  emit vaultAdded(vaultId);
  m_isWorking = false;
}

void VaultManager::onOpenVaultError(uuid vaultId, const QString& errorMessage,
                                    bool stopWorking) noexcept {
  qDebug() << "Failed to open vault with ID" << vaultId.toString().c_str()
           << ":" << errorMessage;
  emit vaultOpenFailed(vaultId, errorMessage);
  if (stopWorking) m_isWorking = false;
}

void VaultManager::onOpenVaultSuccess(uuid vaultId) noexcept {
  qDebug() << "Vault opened successfully with ID" << vaultId.toString().c_str();
  emit vaultOpened(vaultId);
  m_isWorking = false;
}

void VaultManager::onAddItemError(const QString& errorMessage,
                                  bool stopWorking) noexcept {
  qDebug() << "Failed to add item:" << errorMessage;
  emit addItemFailed(errorMessage);
  if (stopWorking) m_isWorking = false;
}

void VaultManager::onAddItemSuccess(uuid vaultId, uuid itemId) noexcept {
  qDebug() << "Item added successfully to vault ID:"
           << vaultId.toString().c_str()
           << "with item ID:" << itemId.toString().c_str();
  emit itemAdded(vaultId, itemId);
  m_isWorking = false;
}

void VaultManager::onOpenItemError(uuid vaultId, uuid itemId,
                                   const QString& errorMessage,
                                   bool stopWorking) noexcept {
  qDebug() << "Failed to open item with ID" << itemId.toString().c_str()
           << "in vault ID" << vaultId.toString().c_str() << ":"
           << errorMessage;
  emit itemOpenFailed(vaultId, itemId, errorMessage);
  if (stopWorking) m_isWorking = false;
}

void VaultManager::onOpenItemSuccess(uuid vaultId,
                                     const VaultItemModel& item) noexcept {
  qDebug() << "Item opened successfully with ID" << item.getId()
           << "in vault ID" << vaultId.toString().c_str();
  emit itemOpened(vaultId, item);
  m_isWorking = false;
}

void VaultManager::onOpenItemSuccess(uuid vaultId,
                                     const VaultItem& item) noexcept {
  VaultItemModel model;
  try {
    model = item.toModel(m_vaults.at(vaultId));
  } catch (const std::exception& e) {
    qWarning() << "Failed to convert item to model for item ID"
               << item.getId().toString().c_str() << "in vault ID"
               << vaultId.toString().c_str() << ":" << e.what();
    onOpenItemError(
        vaultId, item.getId(),
        "Failed to convert item to model: " + QString::fromStdString(e.what()));
    return;
  }
  onOpenItemSuccess(vaultId, model);
}

void VaultManager::onEditItemError(const QString& errorMessage,
                                   bool stopWorking) noexcept {
  qDebug() << "Failed to edit item:" << errorMessage;
  emit editItemFailed(errorMessage);
  if (stopWorking) m_isWorking = false;
}

void VaultManager::onEditItemSuccess(ikea400::uuid vaultId,
                                     const VaultItem& item,
                                     const VaultItemModel& itemModel) noexcept {
  qDebug() << "Item edited successfully with ID" << item.getId().toString()
           << "in vault ID" << vaultId.toString();
  emit itemEdited(vaultId, item, itemModel);
  m_isWorking = false;
}

void VaultManager::onEditItemSuccess(ikea400::uuid vaultId,
                                     const VaultItem& item) noexcept {
  VaultItemModel model;
  try {
    model = item.toModel(m_vaults.at(vaultId));
  } catch (const std::exception& e) {
    qWarning() << "Failed to convert edited item to model for item ID"
               << item.getId().toString() << "in vault ID" << vaultId.toString()
               << ":" << e.what();
    onEditItemError("Failed to convert edited item to model: " +
                    QString::fromStdString(e.what()));
    return;
  }

  onEditItemSuccess(vaultId, item, model);
}
