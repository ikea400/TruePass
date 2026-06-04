#pragma once

#include <QWidget>
#include <QPushButton>

#include "TotpLabel.h"

class TotpCopyButton : public QPushButton {
  Q_OBJECT
 public:
  explicit TotpCopyButton(QWidget* parent = nullptr);
  ~TotpCopyButton();
  void setTotpLabel(TotpLabel* totpLabel);

 public slots:
  void onButtonClicked();

 private:
  TotpLabel* m_totpLabel{nullptr};
};