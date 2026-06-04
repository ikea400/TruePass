#include "TotpCopyButton.h"

#include <QApplication>
#include <QClipboard>

TotpCopyButton::TotpCopyButton(QWidget* parent) : QPushButton(parent) {}

TotpCopyButton::~TotpCopyButton() {}

void TotpCopyButton::setTotpLabel(TotpLabel* totpLabel) {
  if (m_totpLabel) {
    disconnect(this, &QPushButton::clicked, this,
               &TotpCopyButton::onButtonClicked);
  }
  m_totpLabel = totpLabel;
  if (m_totpLabel) {
    connect(this, &QPushButton::clicked, this,
            &TotpCopyButton::onButtonClicked);
  }
}

void TotpCopyButton::onButtonClicked() {
  if (!m_totpLabel) return;

  if (m_totpLabel->hasSecret()) {
    QString totp = m_totpLabel->currentTotp();
    if (!totp.isNull() && !totp.isEmpty()) {
      QClipboard* clipboard = QApplication::clipboard();
      clipboard->setText(totp);
    }
  }
}
