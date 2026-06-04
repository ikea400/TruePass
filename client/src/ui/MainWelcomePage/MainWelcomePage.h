#pragma once

#include <QStackedWidget>
#include <QtWidgets/QMainWindow>

#include "../LoginWidget/LoginWidget.h"
#include "../RegisterWidget/RegisterWidget.h"
#include "ui_MainWelcomePage.h"

class MainWelcomePage : public QMainWindow {
  Q_OBJECT

 public:
  MainWelcomePage(QWidget* parent = nullptr);
  ~MainWelcomePage();

 signals:
  void registerRequested(const QString& username, const QString& email,
                         const QString& password);
  void loginRequested(const QString& username, const QString& password);

 public slots:
  void onAboutToQuit() noexcept;
  void onOpenRegister() noexcept;
  void onOpenLogin() noexcept;
  void onRegistrationFailed(const QString& error);
  void onRegistrationSuccess() noexcept;
  void onLoginFailed(const QString& error);
  void onLoginSuccess() noexcept;

 private:
  Ui::MainWelcomePageClass ui;

  QStackedWidget* m_stackedWidget = nullptr;

  LoginWidget* m_loginWidget = nullptr;
  RegisterWidget* m_registerWidget = nullptr;
};
