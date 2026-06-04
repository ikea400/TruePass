#include "GeneratePasswordDialog.h"

#include <utils/ScopedTimer.h>

#include <QMessageBox>

#include "../../core/PasswordGenerator.h"

GeneratePasswordDialog::GeneratePasswordDialog(QWidget* parent)
    : QDialog(parent) {
  ui.setupUi(this);

  connect(ui.passwordRadioButton, &QRadioButton::toggled, this,
          &GeneratePasswordDialog::onPasswordTypeToggled);
  connect(ui.passphraseRadioButton, &QRadioButton::toggled, this,
          &GeneratePasswordDialog::onPassphraseTypeToggled);
  connect(ui.generatePasswordButton, &QPushButton::clicked, this,
          &GeneratePasswordDialog::generatePassword);
  connect(ui.charactersSlider, &LabeledSlider::valueChanged, this,
          &GeneratePasswordDialog::generatePassword);
  connect(ui.wordsSlider, &LabeledSlider::valueChanged, this,
          &GeneratePasswordDialog::generatePassword);
  connect(ui.uppercaseCheckBox, &QCheckBox::toggled, this,
          &GeneratePasswordDialog::onPasswordSettingsChanged);
  connect(ui.lowercaseCheckBox, &QCheckBox::toggled, this,
          &GeneratePasswordDialog::onPasswordSettingsChanged);
  connect(ui.numberCheckBox, &QCheckBox::toggled, this,
          &GeneratePasswordDialog::onPasswordSettingsChanged);
  connect(ui.specialCheckBox, &QCheckBox::toggled, this,
          &GeneratePasswordDialog::onPasswordSettingsChanged);
  connect(ui.capitalizeCheckBox, &QCheckBox::toggled, this,
          &GeneratePasswordDialog::generatePassword);
  connect(ui.includeNumberCheckBox, &QCheckBox::toggled, this,
          &GeneratePasswordDialog::generatePassword);
  connect(ui.ambiguousCheckBox, &QCheckBox::toggled, this,
          &GeneratePasswordDialog::generatePassword);
  connect(ui.separatorInput, &KeyCaptureWidget::characterChanged, this,
          &GeneratePasswordDialog::generatePassword);
  connect(ui.confirmButton, &QPushButton::clicked, this,
          &GeneratePasswordDialog::onConfirmButtonClicked);
  connect(ui.cancelButton, &QPushButton::clicked, this,
          &GeneratePasswordDialog::onCancelButtonClicked);

  ui.wordsSlider->setLabel(ui.wordsLabel);
  ui.wordsSlider->setFormat("Words: %1");

  ui.charactersSlider->setLabel(ui.charactersLabel);
  ui.charactersSlider->setFormat("Characters: %1");

  ui.copyPasswordButton->setLabel(ui.passwordLabel);

  generatePassword();
}

GeneratePasswordDialog::~GeneratePasswordDialog() {}

void GeneratePasswordDialog::onPasswordTypeToggled(bool checked) {
  if (!checked) return;

  ui.stackedWidget->setCurrentWidget(ui.passwordPage);
  generatePassword();
}
void GeneratePasswordDialog::onPassphraseTypeToggled(bool checked) {
  if (!checked) return;

  ui.stackedWidget->setCurrentWidget(ui.passphrasePage);
  generatePassword();
}
void GeneratePasswordDialog::onPasswordSettingsChanged() {
  generatePassword();

  QList<QCheckBox*> checkBoxes = {ui.uppercaseCheckBox, ui.lowercaseCheckBox,
                                  ui.numberCheckBox, ui.specialCheckBox};

  // Count how many are currently checked
  int checkedCount = 0;
  QCheckBox* lastChecked = nullptr;

  for (QCheckBox* cb : checkBoxes) {
    if (cb->isChecked()) {
      checkedCount++;
      lastChecked = cb;
    }
  }

  // If only one is checked, disable it so it can't be
  // unchecked. Otherwise, ensure all are enabled.
  if (checkedCount == 1 && lastChecked) {
    lastChecked->setEnabled(false);
  } else {
    for (QCheckBox* cb : checkBoxes) {
      cb->setEnabled(true);
    }
  }
}

void GeneratePasswordDialog::onConfirmButtonClicked() {
  emit passwordGenerated(ui.passwordLabel->text());
  close();
}

void GeneratePasswordDialog::onCancelButtonClicked() { close(); }

void GeneratePasswordDialog::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  generatePassword();
}

void GeneratePasswordDialog::generatePassword() {
  QString password;

  try {
    if (ui.passwordRadioButton->isChecked()) {
      int length = ui.charactersSlider->value();
      bool includeUppercase = ui.uppercaseCheckBox->isChecked();
      bool includeLowercase = ui.lowercaseCheckBox->isChecked();
      bool includeNumbers = ui.numberCheckBox->isChecked();
      bool includeSymbols = ui.specialCheckBox->isChecked();
      bool ambiguousCharacters = ui.ambiguousCheckBox->isChecked();

      password = PasswordGenerator::generatePassword(
          length, includeUppercase, includeLowercase, includeNumbers,
          includeSymbols, ambiguousCharacters);
    } else {
      int wordCount = ui.wordsSlider->value();
      bool capitalize = ui.capitalizeCheckBox->isChecked();
      bool includeNumber = ui.includeNumberCheckBox->isChecked();
      QChar separator = ui.separatorInput->currentCharacter();
      password = PasswordGenerator::generatePassphrase(
          wordCount, capitalize, includeNumber, separator);
    }
  } catch (const std::exception& e) {
    QMessageBox::critical(this, "Error", e.what());
  }

  ui.passwordLabel->setText(password);
}
