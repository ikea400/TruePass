#include "KeyCaptureWidget.h"

#include <QKeyEvent>

KeyCaptureWidget::KeyCaptureWidget(QWidget* parent) : QLineEdit(parent) {}

void KeyCaptureWidget::setCurrentCharacter(QChar c) noexcept {
  if (m_currentChar != c) {
    m_currentChar = c;
    emit characterChanged(c);
  }
}

void KeyCaptureWidget::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Backspace) {
    setText("");
    setCurrentCharacter(m_noKeyChar);
    return;
  }

  QString text = event->text();

  if (!text.isEmpty() && text[0].isPrint()) {
    QChar input = text[0];
    setText(input);
    setCurrentCharacter(input);
  }
}
