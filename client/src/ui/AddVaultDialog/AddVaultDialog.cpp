#include "AddVaultDialog.h"

#include <utils/validation.h>

AddVaultDialog::AddVaultDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);

  connect(ui.nameInput, &QLineEdit::editingFinished, this,
          &AddVaultDialog::onNameEditingFinished);
  connect(ui.descriptionInput, &QLineEdit::editingFinished, this,
          &AddVaultDialog::onDescriptionEditingFinished);
  connect(ui.confirmButton, &LoadingButton::loadingStarted, this,
          &AddVaultDialog::onConfirmButtonClicked);

  updateError("");
}

AddVaultDialog::~AddVaultDialog() {}

void AddVaultDialog::onNameEditingFinished() {
  QString name = ui.nameInput->text().trimmed();
  ui.nameInput->setText(
      name.normalized(QString::NormalizationForm::NormalizationForm_C));
}
void AddVaultDialog::onDescriptionEditingFinished() {
  QString description = ui.descriptionInput->text().trimmed();
  ui.descriptionInput->setText(
      description.normalized(QString::NormalizationForm::NormalizationForm_C));
}

void AddVaultDialog::onConfirmButtonClicked() {
  inputValidation();

  if (!ui.errorLabel->text().isEmpty()) {
    ui.confirmButton->stopLoading();
    return;
  }

  emit createVault(ui.nameInput->text(), ui.descriptionInput->text());
}

void AddVaultDialog::onVaultAdded() {
  ui.confirmButton->stopLoading();
  accept();
}

void AddVaultDialog::onAddVaultError(const QString &error) {
  ui.confirmButton->stopLoading();
  updateError(error);
}

void AddVaultDialog::inputValidation() {
  QString name = ui.nameInput->text();
  QString description = ui.descriptionInput->text();
  if (name.isEmpty()) {
    updateError("Name cannot be empty");
    return;
  }

  const auto nameValidationResult =
      validation::validateVaultName(name.toStdString());

  if (!nameValidationResult.isValid()) {
    updateError(QString::fromStdString(nameValidationResult.error()));
    return;
  }

  const auto descriptionValidationResult =
      validation::validateVaultDescription(description.toStdString());
  if (!descriptionValidationResult.isValid()) {
    updateError(QString::fromStdString(descriptionValidationResult.error()));
    return;
  }

  updateError("");
}

void AddVaultDialog::updateError(const QString &error) {
  ui.errorLabel->setText(error);
  ui.errorLabel->setHidden(error.isEmpty());
}
