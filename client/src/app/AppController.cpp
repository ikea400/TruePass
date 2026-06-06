#include "AppController.h"

#include <qapplication.h>
#include <qlibraryinfo.h>
#include <qlogging.h>
#include <qstring.h>
#include <qurl.h>
#include <utils/uuid.h>
#include <utils/validation.h>

#include <memory>
#include <optional>

#include "../core/ClientAuth.h"
#include "../core/VaultItem.h"
#include "../core/VaultManager.h"
#include "../model/VaultItemListModel.h"
#include "../model/VaultItemModel.h"
#include "../model/VaultListModel.h"
#include "../network/ServerSession.h"
#include "../ui/MainVaultPage/MainVaultPage.h"
#include "../ui/MainWelcomePage/MainWelcomePage.h"

AppController::AppController() {
  if (QLibraryInfo::isDebugBuild()) qInfo() << "Debug build of Qt in use.";

  m_serverSession = std::make_unique<ServerSession>();
  m_vaultManager = std::make_unique<VaultManager>(m_serverSession.get());

  m_vaultListModel = new VaultListModel(this);
  m_vaultItemListModel = new VaultItemListModel(this);

  m_mainWelcomePage = new MainWelcomePage();
  m_mainVaultPage = new MainVaultPage();

  m_mainVaultPage->setVaultListModel(m_vaultListModel);
  m_mainVaultPage->setVaultItemListModel(m_vaultItemListModel);

  connect(qApp, &QApplication::aboutToQuit, this,
          &AppController::onAboutToQuit);

  connect(m_mainWelcomePage, &MainWelcomePage::registerRequested, this,
          &AppController::onRegisterRequested);
  connect(m_mainWelcomePage, &MainWelcomePage::loginRequested, this,
          &AppController::onLoginRequested);
  connect(m_serverSession->authService(), &ClientAuth::registrationFailed,
          m_mainWelcomePage, &MainWelcomePage::onRegistrationFailed);
  connect(m_serverSession->authService(), &ClientAuth::registrationSuccess,
          m_mainWelcomePage, &MainWelcomePage::onRegistrationSuccess);
  connect(m_serverSession->authService(), &ClientAuth::loginFailed,
          m_mainWelcomePage, &MainWelcomePage::onLoginFailed);
  connect(m_serverSession->authService(), &ClientAuth::loginSuccess, this,
          &AppController::onLoginSuccess);
  connect(m_mainVaultPage, &MainVaultPage::addVault, this,
          &AppController::onAddVaultRequested);
  connect(m_mainVaultPage, &MainVaultPage::updateVaultList, this,
          &AppController::onUpdateVaultListRequested);
  connect(m_mainVaultPage, &MainVaultPage::openVault, this,
          &AppController::onOpenVaultRequested);
  connect(m_mainVaultPage, &MainVaultPage::addItem, this,
          &AppController::onAddItem);
  connect(m_mainVaultPage, &MainVaultPage::openItem, this,
          &AppController::onOpenItemRequested);
  connect(m_mainVaultPage, &MainVaultPage::editItem, this,
          &AppController::onEditItem);
  connect(m_mainVaultPage, &MainVaultPage::toggleFavorite, this,
          &AppController::onToggleFavorite);

  connect(m_vaultManager.get(), &VaultManager::vaultListUpdated, this,
          &AppController::onVaultListUpdated);
  connect(m_vaultManager.get(), &VaultManager::vaultListUpdateFailed, this,
          &AppController::onVaultListUpdateFailed);
  connect(m_vaultManager.get(), &VaultManager::addVaultFailed, this,
          &AppController::onAddVaultError);
  connect(m_vaultManager.get(), &VaultManager::vaultAdded, this,
          &AppController::onVaultAdded);
  connect(m_vaultManager.get(), &VaultManager::vaultOpened, this,
          &AppController::onVaultOpened);
  connect(m_vaultManager.get(), &VaultManager::vaultOpenFailed, this,
          &AppController::onOpenVaultError);
  connect(m_vaultManager.get(), &VaultManager::addItemFailed, this,
          &AppController::onAddItemError);
  connect(m_vaultManager.get(), &VaultManager::itemAdded, this,
          &AppController::onItemAdded);
  connect(m_vaultManager.get(), &VaultManager::itemOpenFailed, this,
          &AppController::onOpenItemError);
  connect(m_vaultManager.get(), &VaultManager::itemOpened, this,
          &AppController::onItemOpened);
  connect(m_vaultManager.get(), &VaultManager::itemEdited, this,
          &AppController::onItemEdited);
  connect(m_vaultManager.get(), &VaultManager::editItemFailed, this,
          &AppController::onEditItemError);
  connect(m_vaultManager.get(), &VaultManager::favoriteToggled, this,
          &AppController::onFavoriteToggled);
}

AppController::~AppController() {
  delete m_mainWelcomePage;
  delete m_mainVaultPage;
}

void AppController::start() {
  m_serverSession->connectToServer(QUrl("https://localhost:443"));
  showWelcomePage();
}

void AppController::onAboutToQuit() noexcept { m_serverSession.reset(); }

void AppController::onLoginRequested(const QString& identifier,
                                     const QString& password) noexcept {
  if (!validation::validateUsername(identifier.toStdString()).isValid() ||
      !validation::validatePassword(password.toStdString()).isValid()) {
    qWarning() << "Invalid login data.";
    m_mainWelcomePage->onLoginFailed("Invalid username or password.");
    return;
  }

  m_serverSession->authService()->loginUser(identifier, password);
}

void AppController::onRegisterRequested(const QString& username,
                                        const QString& email,
                                        const QString& password) noexcept {
  if (!validation::validateUsername(username.toStdString()).isValid() ||
      !validation::validateEmail(email.toStdString()).isValid() ||
      !validation::validatePassword(password.toStdString()).isValid()) {
    qWarning() << "Invalid registration data.";
    m_mainWelcomePage->onRegistrationFailed("Invalid input values.");
    return;
  }

  m_serverSession->authService()->registerUser(username, email, password);
}

void AppController::onLoginSuccess() noexcept {
  m_mainWelcomePage->onLoginSuccess();
  showVaultPage();
}

void AppController::onUpdateVaultListRequested() noexcept {
  qDebug("Updating vault list...");
  m_vaultManager->updateVaultList();
}

void AppController::onVaultListUpdated() noexcept {
  m_vaultListModel->setVaults(m_vaultManager->getVaults());
  m_mainVaultPage->onVaultListUpdated();
}

void AppController::onVaultListUpdateFailed(const QString& error) {
  m_mainVaultPage->onVaultListUpdateFailed(error);
}

void AppController::onAddVaultRequested(const QString& name,
                                        const QString& description) noexcept {
  if (!validation::validateVaultName(name.toStdString()).isValid()) {
    m_mainVaultPage->onAddVaultError("Invalid vault name.");
    return;
  }
  if (!validation::validateVaultDescription(description.toStdString())
           .isValid()) {
    m_mainVaultPage->onAddVaultError("Invalid vault description.");
    return;
  }

  m_vaultManager->addVault(name, description);
}

void AppController::onVaultAdded(const ikea400::uuid& vaultId) noexcept {
  if (auto it = m_vaultManager->getVaults().find(vaultId);
      it != m_vaultManager->getVaults().end()) {
    m_vaultListModel->addVault(it->second);

    m_mainVaultPage->onVaultAdded(vaultId);
  } else {
    qWarning() << "Added vault not found in vault manager.";
  }
}

void AppController::onAddVaultError(const QString& error) {
  m_mainVaultPage->onAddVaultError(error);
}

void AppController::onOpenVaultRequested(
    const ikea400::uuid& vaultId) noexcept {
  m_vaultItemListModel->clearItems();
  m_vaultManager->openVault(vaultId);
}

void AppController::onVaultOpened(const ikea400::uuid& vaultId) noexcept {
  if (auto it = m_vaultManager->getVaultItems().find(vaultId);
      it != m_vaultManager->getVaultItems().end()) {
    m_vaultItemListModel->setItems(it->second);
  } else {
    qWarning() << "Opened vault items not found in vault manager.";
    onOpenVaultError(vaultId, "Opened vault items not found in vault manager.");
  }
}

void AppController::onOpenVaultError(const ikea400::uuid& vaultId,
                                     const QString& error) {
  m_vaultItemListModel->enterErrorMode(error);
}

void AppController::onAddItem(const VaultItemModel& item,
                              const ikea400::uuid& vaultId) {
  if (!validation::validateVaultItemName(item.getName().toStdString())
           .isValid()) {
    m_mainVaultPage->onAddItemError("Invalid item name.");
    return;
  }

  if (!validation::validateVaultItemNote(item.getNote().toStdString())
           .isValid()) {
    m_mainVaultPage->onAddItemError("Invalid item note.");
    return;
  }

  m_vaultManager->addItem(item, vaultId);
}

void AppController::onAddItemError(const QString& error) {
  m_mainVaultPage->onAddItemError(error);
}

void AppController::onItemAdded(const ikea400::uuid& vaultId,
                                const ikea400::uuid& itemId) {
  std::optional<VaultItem> itemOpt =
      m_vaultManager->getVaultItem(vaultId, itemId);
  if (!itemOpt) {
    qWarning() << "Added item not found in vault manager.";
    onAddItemError("Added item not found in vault manager.");
    return;
  }

  // Add item to the model
  m_vaultItemListModel->addItem(*itemOpt);

  m_mainVaultPage->onItemAdded(vaultId, itemId);
}

void AppController::onOpenItemRequested(const ikea400::uuid& vaultId,
                                        const ikea400::uuid& itemId) noexcept {
  m_vaultManager->openItem(vaultId, itemId);
}

void AppController::onItemOpened(const ikea400::uuid& vaultId,
                                 const VaultItemModel& item) noexcept {
  m_mainVaultPage->onItemOpened(vaultId, item);
}

void AppController::onOpenItemError(const ikea400::uuid& vaultId,
                                    const ikea400::uuid& itemId,
                                    const QString& error) {
  m_mainVaultPage->onOpenItemError(vaultId, itemId, error);
}

void AppController::onEditItem(const VaultItemModel& item,
                               const ikea400::uuid& vaultId) {
  if (!validation::validateVaultItemName(item.getName().toStdString())
           .isValid()) {
    m_mainVaultPage->onEditItemError("Invalid item name.");
    return;
  }

  if (!validation::validateVaultItemNote(item.getNote().toStdString())
           .isValid()) {
    m_mainVaultPage->onEditItemError("Invalid item note.");
    return;
  }

  m_vaultManager->editItem(item, vaultId);
}

void AppController::onEditItemError(const QString& error) {
  m_mainVaultPage->onEditItemError(error);
}

void AppController::onItemEdited(const ikea400::uuid& vaultId,
                                 const VaultItem& item,
                                 const VaultItemModel& itemModel) {
  m_vaultItemListModel->updateItem(item);

  m_mainVaultPage->onItemEdited(vaultId, itemModel);
}

void AppController::onToggleFavorite(const ikea400::uuid& vaultId,
                                     const ikea400::uuid& itemId,
                                     bool isFavorite) {
  if (auto itemOpt = m_vaultManager->getVaultItem(vaultId, itemId); itemOpt) {
    m_vaultItemListModel->updateItem(*itemOpt);
  }
  m_vaultManager->toggleFavorite(vaultId, itemId, isFavorite);
}

void AppController::onFavoriteToggled(const ikea400::uuid& vaultId,
                                      const ikea400::uuid& itemId,
                                      bool isFavorite) {
  m_mainVaultPage->onFavoriteToggled(vaultId, itemId, isFavorite);
}

void AppController::showWelcomePage() noexcept {
  m_mainVaultPage->hide();
  m_mainWelcomePage->show();
}

void AppController::showVaultPage() noexcept {
  m_mainWelcomePage->hide();
  m_mainVaultPage->show();
  m_mainVaultPage->onShow();
}
