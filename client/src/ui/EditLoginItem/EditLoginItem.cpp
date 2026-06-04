#include "EditLoginItem.h"

#include "../../core/utils.h"
#include "../../model/LoginItemDetailModel.h"
#include "../GeneratePasswordDialog/GeneratePasswordDialog.h"

EditLoginItem::EditLoginItem(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);

  connect(ui.usernameInput, &QLineEdit::textChanged, this,
          &EditLoginItem::onUsernameChanged);
  connect(ui.emailInput, &QLineEdit::textChanged, this,
          &EditLoginItem::onEmailChanged);
  connect(ui.passwordInput, &QLineEdit::textChanged, this,
          &EditLoginItem::onPasswordChanged);
  connect(ui.totpSecretInput, &QLineEdit::textChanged, this,
          &EditLoginItem::onTotpSecretChanged);
  connect(ui.websiteInput, &QLineEdit::textChanged, this,
          &EditLoginItem::onWebsiteChanged);
  connect(ui.generatePasswordButton, &QPushButton::clicked, this,
          &EditLoginItem::onGeneratePasswordClicked);

  ui.copyEmailButton->setLineEdit(ui.emailInput);
  ui.copyUsernameButton->setLineEdit(ui.usernameInput);
  ui.copyPasswordButton->setLineEdit(ui.passwordInput);
  ui.copyWebsiteButton->setLineEdit(ui.websiteInput);
  ui.codeLabel->setInput(ui.totpSecretInput);
  ui.copyCodeButton->setTotpLabel(ui.codeLabel);
  ui.revealPasswordButton->setPasswordField(ui.passwordInput);

  ikea400::utils::setupBase32Input(ui.totpSecretInput);
}

EditLoginItem::~EditLoginItem() {}

void EditLoginItem::setModel(LoginItemDetailModel *model) {
  m_model = model;
  if (m_model) {
    ui.usernameInput->setText(m_model->getUsername());
    ui.emailInput->setText(m_model->getEmail());
    ui.passwordInput->setText(m_model->getPassword());
    ui.totpSecretInput->setText(m_model->getTotpSecret());
    ui.websiteInput->setText(m_model->getWebsite());
  } else {
    ui.usernameInput->clear();
    ui.emailInput->clear();
    ui.passwordInput->clear();
    ui.totpSecretInput->clear();
    ui.websiteInput->clear();
  }
}

void EditLoginItem::onUsernameChanged(const QString &username) {
  if (m_model) {
    m_model->setUsername(username);
  }
}

void EditLoginItem::onEmailChanged(const QString &email) {
  if (m_model) {
    m_model->setEmail(email);
  }
}

void EditLoginItem::onPasswordChanged(const QString &password) {
  if (m_model) {
    m_model->setPassword(password);
  }
}

void EditLoginItem::onTotpSecretChanged(const QString &totpSecret) {
  if (m_model) {
    m_model->setTotpSecret(totpSecret);
  }
}

void EditLoginItem::onWebsiteChanged(const QString &website) {
  if (m_model) {
    m_model->setWebsite(website);
  }
}

void EditLoginItem::onGeneratePasswordClicked() {
  if (!m_generatePasswordDialog) {
    m_generatePasswordDialog = new GeneratePasswordDialog(this);
    connect(m_generatePasswordDialog,
            &GeneratePasswordDialog::passwordGenerated, this,
            [this](const QString &password) {
              ui.passwordInput->setText(password);
            });
  }
  m_generatePasswordDialog->show();
}

void EditLoginItem::onClose() {
  if (m_generatePasswordDialog) {
    m_generatePasswordDialog->close();
  }
}
