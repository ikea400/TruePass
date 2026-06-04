#pragma once

#include <QPlainTextEdit>

class CappedPlainTextEdit : public QPlainTextEdit {
  Q_OBJECT
 public:
  explicit CappedPlainTextEdit(QWidget* parent = nullptr);
  void setMaxLength(int maxLength);
  int maxLength() const;

 public slots:
  void onTextChanged();

 private:
  int m_maxLength = 32768;
};