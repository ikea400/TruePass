#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include "HiddenLabel.h"

class CopyButton : public QPushButton {
  Q_OBJECT

 public:
  CopyButton(QWidget *parent = nullptr) noexcept;
  virtual ~CopyButton() noexcept;

  void setLineEdit(QLineEdit *lineEdit) noexcept;
  void setLabel(QLabel *label) noexcept;
  void setLabel(HiddenLabel *label) noexcept;

 public slots:
  void onClicked() noexcept;

 private:
  QLineEdit *m_lineEdit{nullptr};
  QLabel *m_label{nullptr};
  HiddenLabel *m_hiddenLabel{nullptr};
};