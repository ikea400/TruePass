#include "ViewIdentityItem.h"

#include "../../model/IdentityItemDetailModel.h"

ViewIdentityItem::ViewIdentityItem(QWidget* parent) : QWidget(parent) {
  m_ui.setupUi(this);

  m_ui.firstNameCopy->setLabel(m_ui.firstNameLabel);
  m_ui.lastNameCopy->setLabel(m_ui.lastNameLabel);
  m_ui.addressCopy->setLabel(m_ui.addressLabel);
  m_ui.emailCopy->setLabel(m_ui.emailLabel);
  m_ui.dateOfBirthCopy->setLabel(m_ui.dateOfBirthLabel);
  m_ui.usernameCopy->setLabel(m_ui.usernameLabel);
}

ViewIdentityItem::~ViewIdentityItem() {}

void ViewIdentityItem::updateDisplay() noexcept {
  m_ui.firstNameLabel->setText(m_identityDetails.getFirstName());
  m_ui.lastNameLabel->setText(m_identityDetails.getLastName());
  m_ui.addressLabel->setText(m_identityDetails.getAddress());
  m_ui.emailLabel->setText(m_identityDetails.getEmail());
  m_ui.dateOfBirthLabel->setText(m_identityDetails.getDateOfBirth());
  m_ui.usernameLabel->setText(m_identityDetails.getUsername());
}

void ViewIdentityItem::setIdentityDetails(
    const IdentityItemDetailModel& details) {
  m_identityDetails = details;

  updateDisplay();
}
