#pragma once
#include <QLineEdit.h>
#include <qicon.h>
#include <qpushbutton.h>
#include <qtmetamacros.h>

#include "HiddenLabel.h"

class PasswordRevealButton : public QPushButton {
  Q_OBJECT
 public:
  explicit PasswordRevealButton(QWidget* parent = nullptr);

  virtual ~PasswordRevealButton() noexcept = default;

  void setPasswordField(QLineEdit* passwordField) {
    m_passwordField = passwordField;
  }

  void setPasswordField(HiddenLabel* passwordField) {
    m_passwordLabel = passwordField;
  }

 signals:
  void toggled(bool isRevealed);
  void showPassword();
  void hidePassword();

 public slots:
  void onClicked() noexcept;

 private:
  void onStateUpdated();

 private:
  bool m_isRevealed = false;

  QIcon m_showIcon;
  QIcon m_hideIcon;

  QLineEdit* m_passwordField{nullptr};
  HiddenLabel* m_passwordLabel{nullptr};
};
