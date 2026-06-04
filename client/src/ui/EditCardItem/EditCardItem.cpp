#include "EditCardItem.h"

EditCardItem::EditCardItem(QWidget* parent) : QWidget(parent) {
  m_ui.setupUi(this);
  m_model = nullptr;

  connect(m_ui.numberInput, &QLineEdit::textChanged, this,
          [this](const QString& text) {
            if (m_model) {
              m_model->setCardNumber(text);
            }
          });
  connect(m_ui.billingInput, &QLineEdit::textChanged, this,
          [this](const QString& text) {
            if (m_model) {
              m_model->setBillingAddress(text);
            }
          });
  connect(m_ui.cvvInput, &QLineEdit::textChanged, this,
          [this](const QString& text) {
            if (m_model) {
              m_model->setCvv(text);
            }
          });
  connect(m_ui.holderInput, &QLineEdit::textChanged, this,
          [this](const QString& text) {
            if (m_model) {
              m_model->setCardholderName(text);
            }
          });
  connect(m_ui.expirationInput, &QLineEdit::textChanged, this,
          [this](const QString& text) {
            if (m_model) {
              m_model->setExpirationDate(text);
            }
          });
}

EditCardItem::~EditCardItem() {}

void EditCardItem::setModel(CardItemDetailModel* model) {
  m_model = model;

  m_ui.numberInput->setText(m_model ? m_model->getCardNumber() : "");
  m_ui.billingInput->setText(m_model ? m_model->getBillingAddress() : "");
  m_ui.cvvInput->setText(m_model ? m_model->getCvv() : "");
  m_ui.holderInput->setText(m_model ? m_model->getCardholderName() : "");
}
