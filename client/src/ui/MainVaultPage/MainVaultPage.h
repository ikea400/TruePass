#pragma once
#include <utils/uuid.h>

#include <QMainWindow>

#include "../AddItemDialog/AddItemDialog.h"
#include "../AddVaultDialog/AddVaultDialog.h"
#include "ui_MainVaultPage.h"

class VaultItemListProxy;
class VaultItemListModel;
class VaultListModel;
class VaultItem;

class MainVaultPage : public QMainWindow {
  Q_OBJECT

 public:
  MainVaultPage(QWidget* parent = nullptr);
  ~MainVaultPage();

  void setVaultListModel(VaultListModel* model);
  void setVaultItemListModel(VaultItemListModel* model);

 signals:
  void addVault(const QString& name, const QString& description);
  void updateVaultList();
  void openVault(const ikea400::uuid& vaultId);
  void closeVault(const ikea400::uuid& vaultId);
  void addItem(const VaultItemModel& item, const ikea400::uuid& vaultId);
  void openItem(const ikea400::uuid& vaultId, const ikea400::uuid& itemId);
  void editItem(const VaultItemModel& item, const ikea400::uuid& vaultId);

 public slots:
  void onNewVaultButtonClicked();
  void onShow();
  void onSearchTextChanged(const QString& text);
  void onVaultListUpdated();
  void onVaultListUpdateFailed(const QString& error);
  void onVaultAdded(const ikea400::uuid& vaultId);
  void onAddVaultError(const QString& error);
  void onVaultSelected(const QModelIndex& current, const QModelIndex& previous);
  void onAddItemButtonClicked();
  void onAddItem(const VaultItemModel& item);
  void onAddItemError(const QString& error);
  void onItemAdded(const ikea400::uuid& vaultId, const ikea400::uuid& itemId);
  void onItemSelected(const QModelIndex& current, const QModelIndex& previous);
  void onItemOpened(const ikea400::uuid& vaultId, const VaultItemModel& item);
  void onOpenItemError(const ikea400::uuid& vaultId,
                       const ikea400::uuid& itemId, const QString& error);
  void onEditItemError(const QString& error);
  void onItemEdited(const ikea400::uuid& vaultId, const VaultItemModel& item);

  ikea400::uuid getSelectedVaultId() const;

 private:
  Ui::MainVaultPageClass m_ui;

  VaultListModel* m_vaultListModel{nullptr};
  VaultItemListModel* m_vaultItemListModel{nullptr};
  VaultItemListProxy* m_vaultItemListProxy{nullptr};
  AddVaultDialog* m_addVaultDialog{nullptr};
  AddItemDialog* m_addItemDialog{nullptr};
};
