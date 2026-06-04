#include "CopyButton.h"

#include <qapplication.h>
#include <qclipboard.h>

CopyButton::CopyButton(QWidget *parent) noexcept : QPushButton(parent) {
  connect(this, &QPushButton::clicked, this, &CopyButton::onClicked);
}

CopyButton::~CopyButton() noexcept {}

void CopyButton::setLineEdit(QLineEdit *lineEdit) noexcept {
  m_lineEdit = lineEdit;
}

void CopyButton::setLabel(QLabel *label) noexcept { m_label = label; }

void CopyButton::setLabel(HiddenLabel *label) noexcept {
  m_hiddenLabel = label;
}

void CopyButton::onClicked() noexcept {
  if (m_hiddenLabel) {
    QApplication::clipboard()->setText(m_hiddenLabel->text());
  } else if (m_label) {
    QApplication::clipboard()->setText(m_label->text());
  } else if (m_lineEdit) {
    m_lineEdit->selectAll();

    if (m_lineEdit->echoMode() == QLineEdit::Password) {
      // If the line edit is in password mode, we can't use the standart copy
      // method
      QApplication::clipboard()->setText(m_lineEdit->text());
    } else {
      m_lineEdit->copy();
    }
  }
}
