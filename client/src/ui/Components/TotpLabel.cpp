#include "TotpLabel.h"

#include "../../core/utils.h"

TotpLabel::TotpLabel(QWidget* parent)
    : QLabel(parent), m_updateTimer(new QTimer(this)) {
  connect(m_updateTimer, &QTimer::timeout, this, &TotpLabel::updateTotp);

  ikea400::utils::startSyncedTimer(
      m_updateTimer);  // Start the timer synced to the second boundary

  updateTotp();
}

TotpLabel::~TotpLabel() {}

void TotpLabel::setInput(QLineEdit* input) {
  if (m_secretInput) {
    disconnect(m_secretInput, &QLineEdit::textChanged, this,
               &TotpLabel::onSecretInputChanged);
  }
  m_secretInput = input;
  if (m_secretInput) {
    connect(m_secretInput, &QLineEdit::textChanged, this,
            &TotpLabel::onSecretInputChanged);
  }
}

void TotpLabel::setText(const QString& text) {
  // Override to prevent external changes to the text
  qWarning() << "TotpLabel::setText called with: " << text
             << ". This is not allowed.";
}

void TotpLabel::setTotpSecret(const QString& secret) {
  onSecretInputChanged(secret);
}

void TotpLabel::onSecretInputChanged(const QString& newText) {
  m_totpContext.setSecret(newText.toUpper());
  updateTotp();
}

void TotpLabel::updateTotp() {
  if (!m_totpContext.hasSecret()) {
    QLabel::setText(u8"‒‒‒ ‒‒‒");
    return;
  }

  QString newTotp = m_totpContext.generateTotp();
  if (newTotp.isNull() || newTotp.isEmpty()) {
    QLabel::setText(u8"‒‒‒ ‒‒‒");
    return;
  }

  newTotp.insert(3, ' ');  // Format as "123 456"

  QLabel::setText(newTotp);
}
