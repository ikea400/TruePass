#pragma once

#include <QString>

class CardValidator {
 public:
  enum class Provider { Unknown, Visa, Mastercard, Amex };

  static Provider detectProvider(const QString& cardNumber);
  static bool validateLuhn(const QString& cardNumber);
  static QString formatForDisplay(
      const QString& cardNumber);  // Adds spaces every 4 digits

  static bool isVisa(const QString& cardNumber);
  static bool isMastercard(const QString& cardNumber);
  static bool isAmex(const QString& cardNumber);
};