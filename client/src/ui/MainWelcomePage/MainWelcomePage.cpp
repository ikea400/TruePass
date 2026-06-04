#include "MainWelcomePage.h"

MainWelcomePage::MainWelcomePage(QWidget *parent) : QMainWindow(parent) {
  ui.setupUi(this);

  m_stackedWidget = new QStackedWidget(this);

  m_loginWidget = new LoginWidget(this);
  m_registerWidget = new RegisterWidget(this);

  m_stackedWidget->addWidget(m_loginWidget);
  m_stackedWidget->addWidget(m_registerWidget);

  m_stackedWidget->setCurrentWidget(m_loginWidget);

  setCentralWidget(m_stackedWidget);

  QObject::connect(qApp, &QApplication::aboutToQuit, this,
                   &MainWelcomePage::onAboutToQuit);

  QObject::connect(m_loginWidget, &LoginWidget::openRegister, this,
                   &MainWelcomePage::onOpenRegister);

  QObject::connect(m_registerWidget, &RegisterWidget::openLogin, this,
                   &MainWelcomePage::onOpenLogin);
  QObject::connect(m_registerWidget, &RegisterWidget::registerRequested, this,
                   &MainWelcomePage::registerRequested);
  QObject::connect(m_loginWidget, &LoginWidget::loginRequested, this,
                   &MainWelcomePage::loginRequested);
}

MainWelcomePage::~MainWelcomePage() {}

void MainWelcomePage::onAboutToQuit() noexcept {}
void MainWelcomePage::onOpenRegister() noexcept {
  m_stackedWidget->setCurrentWidget(m_registerWidget);
}
void MainWelcomePage::onOpenLogin() noexcept {
  m_stackedWidget->setCurrentWidget(m_loginWidget);
}

void MainWelcomePage::onRegistrationFailed(const QString &error) {
  m_registerWidget->onRegistrationFailed(error);
}

void MainWelcomePage::onRegistrationSuccess() noexcept {
  m_registerWidget->onRegistrationSuccess();
  onOpenLogin();
}

void MainWelcomePage::onLoginFailed(const QString &error) {
  m_loginWidget->onLoginError(error);
}

void MainWelcomePage::onLoginSuccess() noexcept {
  m_loginWidget->onLoginSuccess();
}
