#include "RegisterWidget.h"

#include <utils/validation.h>

RegisterWidget::RegisterWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);

  ui.revealButton->setPasswordField(ui.passwordInput);
  ui.revealConfirmButton->setPasswordField(ui.confirmPasswordInput);

  // Set default values for testing purposes
  ui.usernameInput->setText("User");
  ui.emailInput->setText("user@example.com");
  ui.passwordInput->setText("password12345");
  ui.confirmPasswordInput->setText("password12345");

  QObject::connect(qApp, &QApplication::aboutToQuit, this,
                   &RegisterWidget::onAboutToQuit);
  QObject::connect(ui.loginButton, &QPushButton::clicked, this,
                   &RegisterWidget::openLogin);
  QObject::connect(ui.usernameInput, &QLineEdit::editingFinished, this,
                   &RegisterWidget::onUsernameEditingFinished);
  QObject::connect(ui.emailInput, &QLineEdit::editingFinished, this,
                   &RegisterWidget::onEmailEditingFinished);
  QObject::connect(ui.passwordInput, &QLineEdit::editingFinished, this,
                   &RegisterWidget::onPasswordEditingFinished);
  QObject::connect(ui.confirmPasswordInput, &QLineEdit::editingFinished, this,
                   &RegisterWidget::onConfirmPasswordEditingFinished);
  QObject::connect(ui.registerButton, &LoadingButton::loadingStarted, this,
                   &RegisterWidget::onRegisterClicked);
}

RegisterWidget::~RegisterWidget() {}

void RegisterWidget::onAboutToQuit() noexcept {}

void RegisterWidget::onUsernameEditingFinished() noexcept {
  QString username = ui.usernameInput->text().trimmed();

  auto result = validation::validateUsername(username.toStdString());

  ui.usernameError->setText(QString::fromStdString(result.error()));
}

void RegisterWidget::onEmailEditingFinished() noexcept {
  QString email = ui.emailInput->text().trimmed();

  auto result = validation::validateEmail(email.toStdString());
  ui.emailError->setText(QString::fromStdString(result.error()));
}

void RegisterWidget::onPasswordEditingFinished() noexcept {
  QString password = ui.passwordInput->text();

  auto result = validation::validatePassword(password.toStdString());
  ui.passwordError->setText(QString::fromStdString(result.error()));
}

void RegisterWidget::onConfirmPasswordEditingFinished() noexcept {
  QString confirmPassword = ui.confirmPasswordInput->text().trimmed();
  QString password = ui.passwordInput->text();

  bool valid = confirmPassword == password;

  ui.confirmPasswordError->setText(valid ? "" : "Passwords do not match.");
}

void RegisterWidget::onRegisterClicked() noexcept {
  onUsernameEditingFinished();
  onEmailEditingFinished();
  onPasswordEditingFinished();
  onConfirmPasswordEditingFinished();

  qDebug("Register button clicked. Validating inputs...");
  qDebug("Username Error: %s", qPrintable(ui.usernameError->text()));
  qDebug("Email Error: %s", qPrintable(ui.emailError->text()));
  qDebug("Password Error: %s", qPrintable(ui.passwordError->text()));
  qDebug("Confirm Password Error: %s", qPrintable(ui.confirmPasswordError->text()));

  if (ui.usernameError->text().isEmpty() && ui.emailError->text().isEmpty() &&
      ui.passwordError->text().isEmpty() &&
      ui.confirmPasswordError->text().isEmpty()) {
    emit registerRequested(ui.usernameInput->text().trimmed(),
                           ui.emailInput->text().trimmed(),
                           ui.passwordInput->text());

    clearInputs();
  } else {
    ui.registerButton->stopLoading();
  }

}

void RegisterWidget::onRegistrationFailed(
    const QString &errorMessage) noexcept {
  ui.registerButton->stopLoading();
  updateGlobalError(errorMessage);
}

void RegisterWidget::onRegistrationSuccess() noexcept {
  ui.registerButton->stopLoading();
}

void RegisterWidget::updateGlobalError(const QString &error) noexcept {
  ui.globalError->setText(error);
}
