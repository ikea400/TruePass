#include "PasswordRevealButton.h"

PasswordRevealButton::PasswordRevealButton(QWidget* parent)
    : QPushButton(parent) {
  m_hideIcon.addFile(":/icons/icons/eye-hide.svg");
  m_showIcon.addFile(":/icons/icons/eye-show.svg");

  setIcon(m_showIcon);

  m_isRevealed = false;

  QObject::connect(this, &QPushButton::clicked, this,
                   &PasswordRevealButton::onClicked);
}

void PasswordRevealButton::onClicked() noexcept {
  m_isRevealed = !m_isRevealed;
  emit toggled(m_isRevealed);
  if (m_isRevealed) {
    emit showPassword();
  } else {
    emit hidePassword();
  }

  onStateUpdated();
}

void PasswordRevealButton::onStateUpdated() {
  setIcon(m_isRevealed ? m_hideIcon : m_showIcon);
  if (m_passwordField) {
    m_passwordField->setEchoMode(m_isRevealed ? QLineEdit::Normal
                                              : QLineEdit::Password);
  }
  if (m_passwordLabel) {
    m_passwordLabel->setDisplayMode(!m_isRevealed);
  }
}
