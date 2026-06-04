#include "CardLineEdit.h"

CardLineEdit::CardLineEdit(QWidget* parent)
    : QLineEdit(parent), m_providerIconAction(new QAction(this)) {

  m_providerIconAction->setIcon(QIcon(":/icons/icons/card.svg"));

  addAction(m_providerIconAction, QLineEdit::LeadingPosition);

  connect(this, &QLineEdit::textChanged, this,
          &CardLineEdit::updateProviderIcon);
}

void CardLineEdit::updateProviderIcon(const QString& cardNumber)
{
  CardValidator::Provider provider = CardValidator::detectProvider(cardNumber);
  if (provider != m_currentProvider) {
    m_currentProvider = provider;
    switch (provider) {
      case CardValidator::Provider::Visa:
        m_providerIconAction->setIcon(QIcon(":/icons/icons/visa.svg"));
        break;
      case CardValidator::Provider::Mastercard:
        m_providerIconAction->setIcon(QIcon(":/icons/icons/mastercard.svg"));
        break;
      case CardValidator::Provider::Amex:
        m_providerIconAction->setIcon(QIcon(":/icons/icons/amex.svg"));
        break;
      default:
        m_providerIconAction->setIcon(QIcon(":/icons/icons/card.svg"));
        break;
    }
  }
}
