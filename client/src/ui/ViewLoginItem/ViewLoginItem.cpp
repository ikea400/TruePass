#include "ViewLoginItem.h"

#include "../../model/LoginItemDetailModel.h"

ViewLoginItem::ViewLoginItem(QWidget* parent) : QWidget(parent) {
  m_ui.setupUi(this);

  m_ui.copyUsernameButton->setLabel(m_ui.usernameLabel);
  m_ui.copyEmailButton->setLabel(m_ui.emailLabel);
  m_ui.copyPasswordButton->setLabel(m_ui.passwordLabel);
  m_ui.copyWebsiteButton->setLabel(m_ui.websiteLabel);
  m_ui.copyTotpSecretButton->setLabel(m_ui.totpSecretLabel);
  m_ui.copyTotpCodeButton->setTotpLabel(m_ui.totpCodeLabel);

  m_ui.revealPasswordButton->setPasswordField(m_ui.passwordLabel);
  m_ui.revealTotpSecretButton->setPasswordField(m_ui.totpSecretLabel);
}

ViewLoginItem::~ViewLoginItem() {}

void ViewLoginItem::updateDisplay() noexcept {
  m_ui.usernameLabel->setText(m_loginDetails.getUsername());
  m_ui.passwordLabel->setText(m_loginDetails.getPassword());
  m_ui.emailLabel->setText(m_loginDetails.getEmail());
  m_ui.websiteLabel->setText(m_loginDetails.getWebsite());
  m_ui.totpSecretLabel->setText(m_loginDetails.getTotpSecret());

  m_ui.totpCodeLabel->setTotpSecret(m_loginDetails.getTotpSecret());
}

void ViewLoginItem::setLoginDetails(const LoginItemDetailModel& details) {
  m_loginDetails = details;

  updateDisplay();
}
