#pragma once

#include <QWidget>

#include "ui_LoginWidget.h"

class LoginWidget : public QWidget {
  Q_OBJECT

 public:
  LoginWidget(QWidget* parent = nullptr);
  ~LoginWidget();

  void Reset(const QString& username = QString()) noexcept;
 signals:
  void openRegister(const QString& username);
  void loginRequested(const QString& username, const QString& password);

 public slots:
  void onAboutToQuit() noexcept;
  void onRegisterClicked() noexcept;
  void onLoginClicked() noexcept;
  void onShowPasswordToggled(bool checked) noexcept;
  void onUsernameEditingFinished() noexcept;
  void onPasswordEditingFinished() noexcept;

  void onLoginError(const QString& errorMessage) noexcept;
  void onLoginSuccess() noexcept;

 private:
  void clearErrors(bool clearGlobalMessage) noexcept;

  void setGlobalError(const QString& errorMessage) noexcept;

 private:
  Ui::LoginWidgetClass ui;
};
