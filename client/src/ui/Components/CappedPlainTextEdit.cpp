#include "CappedPlainTextEdit.h"

CappedPlainTextEdit::CappedPlainTextEdit(QWidget* parent)
    : QPlainTextEdit(parent) {
  connect(this, &QPlainTextEdit::textChanged, this,
          &CappedPlainTextEdit::onTextChanged);
}

void CappedPlainTextEdit::setMaxLength(int maxLength) {
  m_maxLength = maxLength;
}

int CappedPlainTextEdit::maxLength() const { return m_maxLength; }

void CappedPlainTextEdit::onTextChanged() {
  QString text = toPlainText();
  if (text.length() > m_maxLength) {
    // Block signals to prevent infinite loop
    blockSignals(true);

    // Truncate the text
    QString truncatedText = text.left(m_maxLength);
    setPlainText(truncatedText);

    // Move cursor to the end
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);

    // Re-enable signals
    blockSignals(false);
  }
}
