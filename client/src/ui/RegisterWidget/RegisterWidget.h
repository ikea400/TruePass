#pragma once

#include <QWidget>

#include "ui_RegisterWidget.h"

class RegisterWidget : public QWidget {
  Q_OBJECT

 public:
  RegisterWidget(QWidget *parent = nullptr);
  ~RegisterWidget();

  void clearInputs() noexcept {
    ui.usernameInput->clear();
    ui.emailInput->clear();
    ui.passwordInput->clear();
    ui.confirmPasswordInput->clear();
  }

 signals:
  void registerRequested(const QString &username, const QString &email,
                         const QString &password);
  void openLogin();

 public slots:
  void onAboutToQuit() noexcept;
  void onUsernameEditingFinished() noexcept;
  void onEmailEditingFinished() noexcept;
  void onPasswordEditingFinished() noexcept;
  void onConfirmPasswordEditingFinished() noexcept;
  void onRegisterClicked() noexcept;
  void onRegistrationFailed(const QString &errorMessage) noexcept;
  void onRegistrationSuccess() noexcept;

 private:
  void updateGlobalError(const QString &error) noexcept;
 private:
  Ui::RegisterWidgetClass ui;
};
