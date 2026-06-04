#include "LoginWidget.h"

#include <utils/validation.h>

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
  ui.setupUi(this);

  QObject::connect(qApp, &QApplication::aboutToQuit, this,
                   &LoginWidget::onAboutToQuit);
  QObject::connect(ui.registerButton, &QPushButton::clicked, this,
                   &LoginWidget::onRegisterClicked);
  QObject::connect(ui.showPassword, &QCheckBox::toggled, this,
                   &LoginWidget::onShowPasswordToggled);
  QObject::connect(ui.usernameInput, &QLineEdit::editingFinished, this,
                   &LoginWidget::onUsernameEditingFinished);
  QObject::connect(ui.passwordInput, &QLineEdit::editingFinished, this,
                   &LoginWidget::onPasswordEditingFinished);
  QObject::connect(ui.loginButton, &QPushButton::clicked, this,
                   &LoginWidget::onLoginClicked);
}

LoginWidget::~LoginWidget() {}

void LoginWidget::Reset(const QString& username) noexcept {
  ui.usernameInput->setText(username);
  ui.passwordInput->clear();
  ui.showPassword->setChecked(false);
}

void LoginWidget::onAboutToQuit() noexcept {}

void LoginWidget::onRegisterClicked() noexcept {
  emit openRegister(ui.usernameInput->text());
}

void LoginWidget::onLoginClicked() noexcept {
  clearErrors(true);

  QString username = ui.usernameInput->text().trimmed();
  QString password = ui.passwordInput->text();

  if (validation::validatePassword(password.toStdString()).isValid() &&
      validation::validateUsername(username.toStdString()).isValid()) {
    emit loginRequested(username, password);
  } else {
    setGlobalError("Invalid username or password");
    ui.loginButton->stopLoading();
  }
}

void LoginWidget::onShowPasswordToggled(bool checked) noexcept {}

void LoginWidget::onUsernameEditingFinished() noexcept {
  QString username = ui.usernameInput->text().trimmed();
  ui.usernameInput->setText(username);
}

void LoginWidget::onPasswordEditingFinished() noexcept {
  ui.passwordInput->setText(ui.passwordInput->text());
}

void LoginWidget::onLoginError(const QString& errorMessage) noexcept {
  setGlobalError(errorMessage);

  ui.loginButton->stopLoading();
}

void LoginWidget::onLoginSuccess() noexcept {
  setGlobalError("");
  ui.loginButton->stopLoading();
}

void LoginWidget::clearErrors(bool clearGlobalMessage) noexcept {
  ui.usernameError->clear();
  ui.passwordError->clear();
  if (clearGlobalMessage) {
    setGlobalError("");
  }
}

void LoginWidget::setGlobalError(const QString& errorMessage) noexcept {
  ui.globalMessage->setText(errorMessage);
}
