#include "ItemInfoWidget.h"

#include <utils/utils.h>

ItemInfoWidget::ItemInfoWidget(QWidget* parent) : QWidget(parent) {
  m_ui.setupUi(this);

  m_editDialog = new EditItemDialog(this);

  connect(m_ui.editButton, &QPushButton::clicked, this,
          &ItemInfoWidget::onEditClicked);
  connect(m_editDialog, &EditItemDialog::itemUpdated, this,
          &ItemInfoWidget::onItemUpdated);

  m_ui.itemNameLabel->setText("");
}

ItemInfoWidget::~ItemInfoWidget() {}

void ItemInfoWidget::openEmptyPage() {
  m_ui.infoStackedWidget->setCurrentWidget(m_ui.emptyPage);
  resetLabel();
  enableButtons(false);
  stopLoadingAnimation();
}
void ItemInfoWidget::openLoadingPage() {
  m_ui.infoStackedWidget->setCurrentWidget(m_ui.loadingPage);
  resetLabel();
  enableButtons(false);
  startLoadingAnimation();
}

void ItemInfoWidget::openErrorPage(const QString& error) {
  m_ui.errorLabel->setText(error);
  m_ui.infoStackedWidget->setCurrentWidget(m_ui.errorPage);
  resetLabel();
  enableButtons(false);
  stopLoadingAnimation();
}

void ItemInfoWidget::openItemInfoPage(const ikea400::uuid& vaultId,
                                      const VaultItemModel& item) {
  m_currentVaultId = vaultId;
  m_currentModel = item;
  m_ui.itemNameLabel->setText(item.getName());
  m_ui.itemNoteEdit->setPlainText(item.getNote());

  const auto visitor = ikea400::utils::overloads{
      [this](const LoginItemDetailModel& loginDetails) {
        m_ui.infoStackedWidget->setCurrentWidget(m_ui.loginPage);
        m_ui.loginPage->setLoginDetails(loginDetails);
      },
      [this](const CardItemDetailModel& cardDetails) {
        m_ui.infoStackedWidget->setCurrentWidget(m_ui.cardPage);
        m_ui.cardPage->setCardDetails(cardDetails);
      },
      [this](const auto&) { openErrorPage("Unsupported item type"); }};

  std::visit(visitor, item.getDetails());

  enableButtons(true);
  stopLoadingAnimation();
}

void ItemInfoWidget::onEditClicked() {
  if (!m_currentModel.has_value()) return;

  m_editDialog->setModel(m_currentModel.value());
  m_editDialog->show();
}

void ItemInfoWidget::onItemUpdated(const VaultItemModel& updatedItem) {
  emit editItem(updatedItem, m_currentVaultId);
}

void ItemInfoWidget::onItemEdited(const ikea400::uuid& vaultId,
                                  const VaultItemModel& item) {
  if (vaultId != m_currentVaultId) return;

  if (!m_currentModel.has_value() || item.getId() != m_currentModel->getId()) {
    return;
  }

  m_currentModel = item;
  updateItemDisplay();

  m_editDialog->onItemEdited();
}

void ItemInfoWidget::onEditItemError(const QString& error) {
  m_editDialog->onEditItemError(error);
}

void ItemInfoWidget::resetLabel() {
  m_ui.itemNameLabel->setText("");
  m_ui.itemNoteEdit->setPlainText("");
}

void ItemInfoWidget::enableButtons(bool enabled) {
  m_ui.favoriteButton->setEnabled(enabled);
  m_ui.editButton->setEnabled(enabled);
}

void ItemInfoWidget::startLoadingAnimation() { m_ui.loadingWidget->start(); }

void ItemInfoWidget::stopLoadingAnimation() { m_ui.loadingWidget->stop(); }

void ItemInfoWidget::updateItemDisplay() {
  if (!m_currentModel.has_value()) {
    openEmptyPage();
    return;
  }

  m_ui.itemNameLabel->setText(m_currentModel->getName());
  m_ui.itemNoteEdit->setPlainText(m_currentModel->getNote());
  const auto visitor = ikea400::utils::overloads{
      [this](const LoginItemDetailModel& loginDetails) {
        m_ui.loginPage->setLoginDetails(loginDetails);
      },
      [this](const CardItemDetailModel& cardDetails) {
        m_ui.cardPage->setCardDetails(cardDetails);
      },
      [this](const auto&) { openErrorPage("Unsupported item type"); }};
  std::visit(visitor, m_currentModel->getDetails());
}
