#include "ViewCardItem.h"

ViewCardItem::ViewCardItem(QWidget* parent) : QWidget(parent) {
  ui.setupUi(this);

  ui.copyNumberButton->setLabel(ui.numberLabel);
  ui.copyCvvButton->setLabel(ui.cvvLabel);
  ui.copyExpirationButton->setLabel(ui.expirationLabel);
  ui.copyHolderLabel->setLabel(ui.holderLabel);
  ui.copyBillingButton->setLabel(ui.billingLabel);
}

ViewCardItem::~ViewCardItem() {}

void ViewCardItem::setCardDetails(const CardItemDetailModel& details) {
  m_cardDetails = details;

  ui.numberLabel->setText(details.getCardNumber());
  ui.cvvLabel->setText(details.getCvv());
  ui.expirationLabel->setText(details.getExpirationDate());
  ui.holderLabel->setText(details.getCardholderName());
  ui.billingLabel->setText(details.getBillingAddress());
}
