#pragma once
#include <QLabel>
#include <QTimer>
#include <QLineEdit>

#include "../../core/TotpContext.h"
#include "HiddenLabel.h"

class TotpLabel : public QLabel {
  Q_OBJECT

 public:
  explicit TotpLabel(QWidget* parent = nullptr);
  ~TotpLabel();

  void setInput(QLineEdit* input);

  bool hasSecret() const { return m_totpContext.hasSecret(); }
  QString currentTotp() const { return m_totpContext.generateTotp(); }

  void setTotpSecret(const QString& secret);

 public slots:
  void setText(const QString&);
  void onSecretInputChanged(const QString& newText);

 private:
  void updateTotp();

 private:
  TotpContext m_totpContext;
  QTimer* m_updateTimer{nullptr};
  QLineEdit* m_secretInput{nullptr};
};