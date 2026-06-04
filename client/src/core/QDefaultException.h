#pragma once
#include <QException>

class QDefaultException : public QException {
 public:
  explicit QDefaultException(const QString &msg) : message(msg.toUtf8()) {}

  void raise() const override { throw *this; }
  QDefaultException *clone() const override {
    return new QDefaultException(*this);
  }

  const char *what() const noexcept override { return message.constData(); }

 private:
  QByteArray message;
};