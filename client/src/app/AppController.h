#pragma once
#include <QObject>

#include "../core/TpmProvider.h"
#include "../core/VaultManager.h"
#include "../model/VaultItemListModel.h"
#include "../model/VaultListModel.h"
#include "../network/ServerSession.h"
#include "../ui/MainVaultPage/MainVaultPage.h"
#include "../ui/MainWelcomePage/MainWelcomePage.h"

class AppController : public QObject {
  Q_OBJECT
 public:
  AppController();
  ~AppController();

  void start();

 public slots:
  void onAboutToQuit() noexcept;
  void onLoginRequested(const QString& identifier,
                        const QString& password) noexcept;
  void onRegisterRequested(const QString& username, const QString& email,
                           const QString& password) noexcept;
  void onLoginSuccess() noexcept;
  void onUpdateVaultListRequested() noexcept;
  void onVaultListUpdated() noexcept;
  void onVaultListUpdateFailed(const QString& error);
  void onAddVaultRequested(const QString& name,
                           const QString& description) noexcept;
  void onVaultAdded(const ikea400::uuid& vaultId) noexcept;
  void onAddVaultError(const QString& error);
  void onOpenVaultRequested(const ikea400::uuid& vaultId) noexcept;
  void onVaultOpened(const ikea400::uuid& vaultId) noexcept;
  void onOpenVaultError(const ikea400::uuid& vaultId, const QString& error);
  void onAddItem(const VaultItemModel& item, const ikea400::uuid& vaultId);
  void onAddItemError(const QString& error);
  void onItemAdded(const ikea400::uuid& vaultId, const ikea400::uuid& itemId);
  void onOpenItemRequested(const ikea400::uuid& vaultId,
                           const ikea400::uuid& itemId) noexcept;
  void onItemOpened(const ikea400::uuid& vaultId,
                    const VaultItemModel& item) noexcept;
  void onOpenItemError(const ikea400::uuid& vaultId,
                       const ikea400::uuid& itemId, const QString& error);
  void onEditItem(const VaultItemModel& item, const ikea400::uuid& vaultId);
  void onEditItemError(const QString& error);
  void onItemEdited(const ikea400::uuid& vaultId, const VaultItem& item,
                    const VaultItemModel& itemModel);

 private:
  void showWelcomePage() noexcept;
  void showVaultPage() noexcept;

 private:
  MainWelcomePage* m_mainWelcomePage;
  MainVaultPage* m_mainVaultPage;
  VaultListModel* m_vaultListModel;
  VaultItemListModel* m_vaultItemListModel;
  std::unique_ptr<ServerSession> m_serverSession;
  std::unique_ptr<VaultManager> m_vaultManager;
  TpmProvider m_tpmProvider;
};