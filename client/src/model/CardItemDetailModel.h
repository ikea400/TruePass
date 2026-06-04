#pragma once

#include <QString>
#include <QStringList>

class CardItemDetailModel {
 public:
  const QString& getCardNumber() const noexcept { return m_cardNumber; }
  const QString& getCardholderName() const noexcept { return m_cardholderName; }
  const QString& getCvv() const noexcept { return m_cvv; }
  const QString& getBillingAddress() const noexcept { return m_billingAddress; }
  const QString getExpirationDate() const noexcept {
    return QString("%1/%2")
        .arg(m_expirationMonth, 2, 10, QChar('0'))
        .arg(m_expirationYear);
  }
  const int getExpirationMonth() const noexcept { return m_expirationMonth; }
  const int getExpirationYear() const noexcept { return m_expirationYear; }

  void setCardNumber(const QString& cardNumber) { m_cardNumber = cardNumber; }
  void setCardholderName(const QString& cardholderName) {
    m_cardholderName = cardholderName;
  }
  void setCvv(const QString& cvv) { m_cvv = cvv; }
  void setBillingAddress(const QString& billingAddress) {
    m_billingAddress = billingAddress;
  }

  bool setExpirationDate(const QString& expirationDate) {
    QStringList parts = expirationDate.split('/');
    if (parts.size() != 2) {
      return false;  // Invalid format
    }
    bool monthOk, yearOk;
    int month = parts[0].toInt(&monthOk);
    int year = parts[1].toInt(&yearOk);
    if (!monthOk || !yearOk) {
      return false;  // Invalid month or year
    }
    return setExpirationDate(month, year);
  }

  bool setExpirationDate(int month, int year) {
    if (month < 1 || month > 12 || year < 0) {
      return false;  // Invalid month or year
    }
    m_expirationMonth = month;
    m_expirationYear = year;
    return true;
  }

 private:
  QString m_cardNumber;
  QString m_cardholderName;
  QString m_cvv;
  QString m_billingAddress;
  int m_expirationMonth;
  int m_expirationYear;
};