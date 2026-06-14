#include "EditItemDialog.h"

#include <qdialog.h>
#include <qstring.h>
#include <qwidget.h>
#include <utils/utils.h>
#include <utils/validation.h>

#include <variant>

#include "../../model/LoginItemDetailModel.h"
#include "../../model/IdentityItemDetailModel.h"
#include "../../model/NoteItemDetailModel.h"
#include "../../model/VaultItemModel.h"

using namespace ikea400;

EditItemDialog::EditItemDialog(QWidget* parent) : QDialog(parent) {
  ui.setupUi(this);

  connect(ui.confirmButton, &QPushButton::clicked, this,
          &EditItemDialog::onConfirm);
  connect(ui.cancelButton, &QPushButton::clicked, this,
          &EditItemDialog::onCancel);

  ui.nameInput->setMaxLength(validation::kMaxVaultItemNameLength);
  ui.noteInput->setMaxLength(validation::kMaxVaultItemNoteLength);
}

EditItemDialog::~EditItemDialog() {}

void EditItemDialog::setModel(const VaultItemModel& model) {
  m_model = model;
  ui.nameInput->setText(model.getName());
  ui.noteInput->setPlainText(model.getNote());

  const auto visitor = ikea400::utils::overloads{
      [this](LoginItemDetailModel& loginDetails) {
        ui.itemStackedWidget->setCurrentWidget(ui.loginPage);
        ui.loginPage->setModel(&loginDetails);
      },
      [this](CardItemDetailModel& cardDetails) {
        ui.itemStackedWidget->setCurrentWidget(ui.cardPage);
        ui.cardPage->setModel(&cardDetails);
      },
      [this](IdentityItemDetailModel& identityDetails) {
        ui.itemStackedWidget->setCurrentWidget(ui.identityPage);
        ui.identityPage->setModel(&identityDetails);
      },
      [this](NoteItemDetailModel& noteDetails) {
        ui.itemStackedWidget->setCurrentWidget(ui.notePage);
        ui.notePage->setModel(&noteDetails);
      }};

  std::visit(visitor, m_model.getDetails());

  updateError({});
}

void EditItemDialog::onConfirm() {
  QString name = ui.nameInput->text().trimmed();

  QString validationError = validateName(name);
  if (!validationError.isEmpty()) {
    updateError(validationError);
    return;
  }

  QString note = ui.noteInput->toPlainText().trimmed();
  m_model.setName(name);
  m_model.setNote(note);

  emit itemUpdated(m_model);
}

void EditItemDialog::onCancel() { close(); }

void EditItemDialog::onItemEdited() { close(); }

void EditItemDialog::onEditItemError(const QString& error) {
  updateError(error);
}

QString EditItemDialog::validateName(const QString& name) {
  const auto validationResult =
      validation::validateVaultItemName(name.toStdString());
  if (validationResult.isValid()) {
    return {};
  } else {
    return QString::fromStdString(validationResult.error());
  }
}

void EditItemDialog::updateError(const QString& error) {
  ui.errorLabel->setText(error);
  ui.errorWidget->setHidden(error.isEmpty());
}
