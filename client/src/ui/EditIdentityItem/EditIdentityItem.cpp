#include "EditIdentityItem.h"

#include <QApplication>
#include <QClipboard>
#include <QDate>

#include "../../model/IdentityItemDetailModel.h"

EditIdentityItem::EditIdentityItem(QWidget* parent) : QWidget(parent) {
  m_ui.setupUi(this);
  m_model = nullptr;

  m_ui.firstNameCopy->setLineEdit(m_ui.firstNameInput);
  m_ui.lastNameCopy->setLineEdit(m_ui.lastNameInput);
  m_ui.addressCopy->setLineEdit(m_ui.addressInput);
  m_ui.emailCopy->setLineEdit(m_ui.emailInput);
  m_ui.usernameCopy->setLineEdit(m_ui.usernameInput);

  connect(m_ui.firstNameInput, &QLineEdit::textChanged, this,
          [this](const QString& text) {
            if (m_model) {
              m_model->setFirstName(text);
            }
          });
  connect(m_ui.lastNameInput, &QLineEdit::textChanged, this,
          [this](const QString& text) {
            if (m_model) {
              m_model->setLastName(text);
            }
          });
  connect(m_ui.addressInput, &QLineEdit::textChanged, this,
          [this](const QString& text) {
            if (m_model) {
              m_model->setAddress(text);
            }
          });
  connect(m_ui.emailInput, &QLineEdit::textChanged, this,
          [this](const QString& text) {
            if (m_model) {
              m_model->setEmail(text);
            }
          });
  connect(m_ui.usernameInput, &QLineEdit::textChanged, this,
          [this](const QString& text) {
            if (m_model) {
              m_model->setUsername(text);
            }
          });
  connect(m_ui.bodInput, &QDateEdit::dateChanged, this,
          [this](const QDate& date) {
            if (m_model) {
              m_model->setDateOfBirth(date.toString(Qt::ISODate));
            }
          });

  connect(m_ui.daoCopy, &QPushButton::clicked, this, [this]() {
    QApplication::clipboard()->setText(m_ui.bodInput->date().toString("yyyy-MM-dd"));
  });
}

EditIdentityItem::~EditIdentityItem() {}

void EditIdentityItem::setModel(IdentityItemDetailModel* model) {
  m_model = model;

  if (m_model) {
    m_ui.firstNameInput->setText(m_model->getFirstName());
    m_ui.lastNameInput->setText(m_model->getLastName());
    m_ui.addressInput->setText(m_model->getAddress());
    m_ui.emailInput->setText(m_model->getEmail());
    m_ui.usernameInput->setText(m_model->getUsername());
    if (!m_model->getDateOfBirth().isEmpty()) {
      m_ui.bodInput->setDate(QDate::fromString(m_model->getDateOfBirth(), Qt::ISODate));
    } else {
      m_ui.bodInput->setDate(QDate::currentDate());
    }
  } else {
    m_ui.firstNameInput->clear();
    m_ui.lastNameInput->clear();
    m_ui.addressInput->clear();
    m_ui.emailInput->clear();
    m_ui.usernameInput->clear();
    m_ui.bodInput->setDate(QDate::currentDate());
  }
}
