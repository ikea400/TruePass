#include "ItemInfoWidget.h"

#include <utils/utils.h>
#include "../../model/IdentityItemDetailModel.h"
#include "../../model/NoteItemDetailModel.h"

ItemInfoWidget::ItemInfoWidget(QWidget* parent) : QWidget(parent) {
  m_ui.setupUi(this);

  m_editDialog = new EditItemDialog(this);

  connect(m_ui.editButton, &QPushButton::clicked, this,
          &ItemInfoWidget::onEditClicked);
  connect(m_ui.favoriteButton, &QPushButton::clicked, this,
          &ItemInfoWidget::onFavoriteClicked);
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
  updateFavoriteButtonIcon(false);
}
void ItemInfoWidget::openLoadingPage() {
  m_ui.infoStackedWidget->setCurrentWidget(m_ui.loadingPage);
  resetLabel();
  enableButtons(false);
  startLoadingAnimation();
  updateFavoriteButtonIcon(false);
}

void ItemInfoWidget::openErrorPage(const QString& error) {
  m_ui.errorLabel->setText(error);
  m_ui.infoStackedWidget->setCurrentWidget(m_ui.errorPage);
  resetLabel();
  enableButtons(false);
  stopLoadingAnimation();
  updateFavoriteButtonIcon(false);
}

void ItemInfoWidget::openItemInfoPage(const ikea400::uuid& vaultId,
                                      const VaultItemModel& item) {
  m_currentVaultId = vaultId;
  m_currentModel = item;
  m_ui.itemNameLabel->setText(item.getName());
  m_ui.itemNoteEdit->setPlainText(item.getNote());
  updateFavoriteButtonIcon(item.isFavorite());

  const auto visitor = ikea400::utils::overloads{
      [this](const LoginItemDetailModel& loginDetails) {
        m_ui.infoStackedWidget->setCurrentWidget(m_ui.loginPage);
        m_ui.loginPage->setLoginDetails(loginDetails);
      },
      [this](const CardItemDetailModel& cardDetails) {
        m_ui.infoStackedWidget->setCurrentWidget(m_ui.cardPage);
        m_ui.cardPage->setCardDetails(cardDetails);
      },
      [this](const IdentityItemDetailModel& identityDetails) {
        m_ui.infoStackedWidget->setCurrentWidget(m_ui.identityPage);
        m_ui.identityPage->setIdentityDetails(identityDetails);
      },
      [this](const NoteItemDetailModel& noteDetails) {
        m_ui.infoStackedWidget->setCurrentWidget(m_ui.notePage);
        m_ui.notePage->setNoteDetails(noteDetails);
      }};

  std::visit(visitor, item.getDetails());

  enableButtons(true);
  stopLoadingAnimation();
}

void ItemInfoWidget::onEditClicked() {
  if (!m_currentModel.has_value()) return;

  m_editDialog->setModel(m_currentModel.value());
  m_editDialog->show();
}

void ItemInfoWidget::onFavoriteClicked() {
  if (!m_currentModel.has_value()) return;

  const auto& item = m_currentModel.value();

  ikea400::uuid itemId =
      ikea400::uuid::fromString<false>(item.getId().toStdString());
  if (itemId.isNull()) {
    qWarning() << "Invalid item ID for favorite toggle";
    return;
  }

  bool newFavoriteState = !item.isFavorite();
  m_currentModel->setIsFavorite(newFavoriteState);
  updateFavoriteButtonIcon(newFavoriteState);

  emit toggleFavorite(m_currentVaultId, itemId, newFavoriteState);
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

void ItemInfoWidget::onFavoriteToggled(const ikea400::uuid& vaultId,
                                       const ikea400::uuid& itemId,
                                       bool isFavorite) {
  if (vaultId != m_currentVaultId) return;

  if (!m_currentModel.has_value() ||
      m_currentModel->getId() != QString::fromStdString(itemId.toString())) {
    return;
  }

  // Update the model with the new favorite state
  m_currentModel->setIsFavorite(isFavorite);
  updateFavoriteButtonIcon(isFavorite);
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
  updateFavoriteButtonIcon(m_currentModel->isFavorite());
  const auto visitor = ikea400::utils::overloads{
      [this](const LoginItemDetailModel& loginDetails) {
        m_ui.loginPage->setLoginDetails(loginDetails);
      },
      [this](const CardItemDetailModel& cardDetails) {
        m_ui.cardPage->setCardDetails(cardDetails);
      },
      [this](const IdentityItemDetailModel& identityDetails) {
        m_ui.identityPage->setIdentityDetails(identityDetails);
      },
      [this](const NoteItemDetailModel& noteDetails) {
        m_ui.notePage->setNoteDetails(noteDetails);
      }};
  std::visit(visitor, m_currentModel->getDetails());
}

void ItemInfoWidget::updateFavoriteButtonIcon(bool isFavorite) {
  QIcon icon;
  if (isFavorite) {
    icon.addFile(QString::fromUtf8(":/icons/icons/star-full.svg"), QSize(),
                 QIcon::Mode::Normal, QIcon::State::Off);
  } else {
    icon.addFile(QString::fromUtf8(":/icons/icons/star.svg"), QSize(),
                 QIcon::Mode::Normal, QIcon::State::Off);
  }
  m_ui.favoriteButton->setIcon(icon);
}
