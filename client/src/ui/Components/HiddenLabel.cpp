#include "HiddenLabel.h"

#include <QStyleOptionFrame>

HiddenLabel::HiddenLabel(QWidget* parent) : QLabel(parent) {
  updateDisplay();
  ;

  m_passwordCharacter =
      char16_t(style()->styleHint(QStyle::SH_LineEdit_PasswordCharacter));
}

HiddenLabel::~HiddenLabel() noexcept {}

void HiddenLabel::setText(const QString& text) {
  m_text = text;
  updateDisplay();
}

QString HiddenLabel::text() const { return m_text; }

void HiddenLabel::setDisplayMode(bool hidden) noexcept {
  m_isHidden = hidden;
  updateDisplay();
}

void HiddenLabel::updateDisplay() noexcept {
  QString displayText =
      m_isHidden ? QString(m_text.length(), m_passwordCharacter) : m_text;
  QLabel::setText(displayText);
}
