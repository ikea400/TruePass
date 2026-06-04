#include "MainVaultPage.h"

#include <utils/ScopedTimer.h>

#include <QAbstractItemModelTester>
#include <QMessageBox>

#include "../../model/VaultItemListModel.h"
#include "../../model/VaultItemListProxy.h"
#include "../../model/VaultListModel.h"
#include "../AddItemDialog/AddItemDialog.h"
#include "../AddVaultDialog/AddVaultDialog.h"

using namespace ikea400;

MainVaultPage::MainVaultPage(QWidget* parent) : QMainWindow(parent) {
  m_ui.setupUi(this);

  m_ui.itemWidget->openEmptyPage();

  m_vaultItemListProxy = new VaultItemListProxy(this);
  m_ui.itemsListView->setModel(m_vaultItemListProxy);

  connect(m_ui.searchInput, &QLineEdit::textChanged, this,
          &MainVaultPage::onSearchTextChanged);
  connect(m_ui.newVaultButton, &QPushButton::clicked, this,
          &MainVaultPage::onNewVaultButtonClicked);
  connect(m_ui.newItemButton, &QPushButton::clicked, this,
          &MainVaultPage::onAddItemButtonClicked);
  connect(m_ui.itemWidget, &ItemInfoWidget::editItem, this,
          &MainVaultPage::editItem);

  setAttribute(Qt::WA_WState_ExplicitShowHide, true);
}

MainVaultPage::~MainVaultPage() {}

void MainVaultPage::setVaultListModel(VaultListModel* model) {
  m_vaultListModel = model;
  m_ui.vaultListView->setModel(m_vaultListModel);

  connect(m_ui.vaultListView->selectionModel(),
          &QItemSelectionModel::currentChanged, this,
          &MainVaultPage::onVaultSelected);
}

void MainVaultPage::setVaultItemListModel(VaultItemListModel* model) {
  new QAbstractItemModelTester(
      model, QAbstractItemModelTester::FailureReportingMode::Fatal, this);

  m_vaultItemListModel = model;
  m_vaultItemListProxy->setSourceModel(m_vaultItemListModel);

  connect(m_ui.itemsListView->selectionModel(),
          &QItemSelectionModel::currentChanged, this,
          &MainVaultPage::onItemSelected);
}

void MainVaultPage::onShow() { emit updateVaultList(); }

void MainVaultPage::onSearchTextChanged(const QString& text) {
  m_vaultItemListProxy->setSearchQuery(text);
}

void MainVaultPage::onVaultListUpdated() {}

void MainVaultPage::onVaultListUpdateFailed(const QString& error) {
  qDebug() << "Vault list update failed:" << error;

  QMessageBox::critical(this, "Error", "Failed to update vault list: " + error);
}

void MainVaultPage::onVaultAdded(const uuid& vaultId) {
  if (m_addVaultDialog) {
    m_addVaultDialog->onVaultAdded();
    m_addVaultDialog->deleteLater();
    m_addVaultDialog = nullptr;
  }
}

void MainVaultPage::onAddVaultError(const QString& error) {
  if (m_addVaultDialog) {
    m_addVaultDialog->onAddVaultError(error);
  }
}

void MainVaultPage::onVaultSelected(const QModelIndex& current,
                                    const QModelIndex& previous) {
  m_ui.itemWidget->openEmptyPage();
  if (previous.isValid()) {
    qDebug() << "Vault deselected:" << previous.data(Qt::UserRole).toString();
    emit closeVault(uuid::fromString<false>(
        previous.data(Qt::UserRole).toString().toStdString()));
  }

  if (!current.isValid()) return;

  qDebug() << "Vault selected:" << current.data(Qt::UserRole).toString();
  emit openVault(uuid::fromString<false>(
      current.data(Qt::UserRole).toString().toStdString()));
}

void MainVaultPage::onAddItemButtonClicked() {
  if (!m_ui.vaultListView->currentIndex().isValid()) {
    QMessageBox::warning(this, "Warning", "Please select a vault first.");
    return;
  }

  if (!m_addItemDialog) {
    m_addItemDialog = new AddItemDialog(this);
    connect(m_addItemDialog, &AddItemDialog::addItem, this,
            &MainVaultPage::onAddItem);
  }

  m_addItemDialog->show();
}

void MainVaultPage::onAddItem(const VaultItemModel& item) {
  qDebug() << "Adding item:" << item.getName();
  uuid vaultId = uuid::fromString<false>(m_ui.vaultListView->currentIndex()
                                             .data(Qt::UserRole)
                                             .toString()
                                             .toStdString());
  if (vaultId.isNull()) {
    QMessageBox::critical(this, "Error", "Invalid vault selected.");
    return;
  }
  emit addItem(item, vaultId);
}

void MainVaultPage::onAddItemError(const QString& error) {
  if (m_addItemDialog) {
    m_addItemDialog->onAddItemError(error);
  }
}

void MainVaultPage::onItemAdded(const uuid& vaultId, const uuid& itemId) {
  if (m_addItemDialog) {
    m_addItemDialog->onItemAdded();
    m_addItemDialog->deleteLater();
    m_addItemDialog = nullptr;
  }

  // Open the item info page for the newly added item
}

void MainVaultPage::onItemSelected(const QModelIndex& current,
                                   const QModelIndex& previous) {
  m_ui.itemWidget->openEmptyPage();

  uuid vaultUid = getSelectedVaultId();
  if (vaultUid.isNull()) {
    qDebug() << "No vault selected, cannot open item.";
    return;
  }

  if (!current.isValid()) return;

  m_ui.itemWidget->openLoadingPage();

  emit openItem(vaultUid,
                uuid::fromString<false>(
                    current.data(Qt::UserRole).toString().toStdString()));

  qDebug() << "Item selected:" << current.data(Qt::UserRole).toString();
}

void MainVaultPage::onItemOpened(const ikea400::uuid& vaultId,
                                 const VaultItemModel& item) {
  m_ui.itemWidget->openItemInfoPage(vaultId, item);
}

void MainVaultPage::onOpenItemError(const ikea400::uuid& vaultId,
                                    const ikea400::uuid& itemId,
                                    const QString& error) {
  qDebug() << "Failed to open item:" << error;
  m_ui.itemWidget->openErrorPage("Failed to open item: " + error);
}

void MainVaultPage::onEditItemError(const QString& error) {
  m_ui.itemWidget->onEditItemError(error);
}

void MainVaultPage::onItemEdited(const ikea400::uuid& vaultId,
                                 const VaultItemModel& item) {
  if (vaultId == getSelectedVaultId()) {
    m_ui.itemWidget->onItemEdited(vaultId, item);
  }
}

ikea400::uuid MainVaultPage::getSelectedVaultId() const {
  return uuid::fromString<false>(m_ui.vaultListView->currentIndex()
                                     .data(Qt::UserRole)
                                     .toString()
                                     .toStdString());
}

void MainVaultPage::onNewVaultButtonClicked() {
  if (!m_addVaultDialog) {
    m_addVaultDialog = new AddVaultDialog(this);
    connect(m_addVaultDialog, &AddVaultDialog::createVault, this,
            &MainVaultPage::addVault);
  }
  m_addVaultDialog->show();
}
