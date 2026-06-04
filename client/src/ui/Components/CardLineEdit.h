#pragma once

#include <QLineEdit>

#include "../../core/CardValidator.h"

class CardLineEdit : public QLineEdit {
  Q_OBJECT
 public:
  explicit CardLineEdit(QWidget* parent = nullptr);

 public slots:
  void updateProviderIcon(const QString& cardNumber);

 private:
  QAction* m_providerIconAction;
  CardValidator::Provider m_currentProvider = CardValidator::Provider::Unknown;
};