#include "AddItemDialog.h"

#include <utils/validation.h>

#include "../../model/LoginItemDetailModel.h"
#include "../../model/NoteItemDetailModel.h"

enum class VaultItemType { Login, Card, Identity, Note };
Q_DECLARE_METATYPE(VaultItemType);

AddItemDialog::AddItemDialog(QWidget* parent) : QDialog(parent) {
  ui.setupUi(this);

  connect(this, &AddItemDialog::closing, ui.loginPage, &EditLoginItem::onClose);

  connect(ui.itemTypeBox, &QComboBox::currentIndexChanged, this,
          &AddItemDialog::onCurrentIndexChanged);
  connect(ui.confirmButton, &LoadingButton::loadingStarted, this,
          &AddItemDialog::onConfirmClicked);
  connect(ui.itemNameInput, &QLineEdit::textChanged, this,
          &AddItemDialog::onNameChanged);
  connect(ui.itemNameInput, &QLineEdit::editingFinished, this,
          &AddItemDialog::onNameEdited);
  connect(ui.noteInput, &QPlainTextEdit::textChanged, this,
          &AddItemDialog::onNoteChanged);

  ui.stackedWidget->setCurrentWidget(ui.loginPage);

  ui.itemTypeBox->addItem("Login", QVariant::fromValue(VaultItemType::Login));
  ui.itemTypeBox->addItem("Card", QVariant::fromValue(VaultItemType::Card));
  ui.itemTypeBox->addItem("Identity",
                          QVariant::fromValue(VaultItemType::Identity));
  ui.itemTypeBox->addItem("Note", QVariant::fromValue(VaultItemType::Note));

  ui.itemNameInput->setMaxLength(validation::kMaxVaultItemNameLength);
  ui.noteInput->setMaxLength(validation::kMaxVaultItemNoteLength);
}

AddItemDialog::~AddItemDialog() {}

void AddItemDialog::onCurrentIndexChanged(int index) {
  if (index < 0) {
    qDebug() << "Item type deselected.";
    return;
  }

  QVariant itemData = ui.itemTypeBox->itemData(index);
  if (!itemData.isValid()) {
    qWarning() << "Invalid item type selected!";
    return;
  }

  VaultItemType type = itemData.value<VaultItemType>();
  switch (type) {
    case VaultItemType::Login:
      m_model.resetType<LoginItemDetailModel>();
      ui.stackedWidget->setCurrentWidget(ui.loginPage);
      ui.loginPage->setModel(m_model.getDetails<LoginItemDetailModel>());
      qDebug() << "Login item type selected.";
      break;
    case VaultItemType::Card:
      m_model.resetType<CardItemDetailModel>();
      ui.stackedWidget->setCurrentWidget(ui.cardPage);
      ui.cardPage->setModel(m_model.getDetails<CardItemDetailModel>());
      qDebug() << "Card item type selected.";
      break;
    case VaultItemType::Identity:
      m_model.resetType<IdentityItemDetailModel>();
      ui.stackedWidget->setCurrentWidget(ui.identityPage);
      ui.identityPage->setModel(m_model.getDetails<IdentityItemDetailModel>());
      qDebug() << "Identity item type selected.";
      break;
    case VaultItemType::Note:
      m_model.resetType<NoteItemDetailModel>();
      ui.stackedWidget->setCurrentWidget(ui.notePage);
      ui.notePage->setModel(m_model.getDetails<NoteItemDetailModel>());
      qDebug() << "Note item type selected.";
      break;
    default:
      qWarning() << "Unknown item type selected!";
      break;
  }
}

void AddItemDialog::onConfirmClicked() {
  validateName();

  if (!ui.nameError->text().isEmpty()) {
    ui.confirmButton->stopLoading();
    return;
  }

  qDebug() << "Creating new item with name: " << m_model.getName();
  m_model.generateId();
  emit addItem(m_model);
}

void AddItemDialog::onNameChanged(const QString& name) {
  if (!ui.nameError->text().isEmpty()) validateName();

  m_model.setName(ui.itemNameInput->text().trimmed());
}

void AddItemDialog::onNameEdited() {
  ui.itemNameInput->setText(ui.itemNameInput->text().trimmed());
  validateName();
}

void AddItemDialog::onNoteChanged() {
  QString note = ui.noteInput->toPlainText().trimmed();
  m_model.setNote(note);
}

void AddItemDialog::onAddItemError(const QString& error) {
  updateGlobalError(error);
  ui.confirmButton->stopLoading();
}

void AddItemDialog::onItemAdded() {
  updateGlobalError("");
  ui.confirmButton->stopLoading();
  accept();
}

void AddItemDialog::validateName() {
  QString name = ui.itemNameInput->text().trimmed();
  if (name.isEmpty()) {
    updateNameError("Name cannot be empty.");
    return;
  }

  auto result = validation::validateVaultItemName(name.toStdString());
  if (!result.isValid())
    updateNameError(QString::fromStdString(result.error()));
  else
    updateNameError("");
}

void AddItemDialog::updateNameError(const QString& error) {
  ui.nameError->setText(error);
}

void AddItemDialog::updateGlobalError(const QString& error) {
  ui.globalError->setText(error);
}

void AddItemDialog::closeEvent(QCloseEvent* e) {
  QDialog::closeEvent(e);
  m_model = VaultItemModel();

  emit closing();
}
