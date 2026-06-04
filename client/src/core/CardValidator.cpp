#include "CardValidator.h"

CardValidator::Provider CardValidator::detectProvider(
    const QString& cardNumber) {
  if (isVisa(cardNumber)) {
    return Provider::Visa;
  } else if (isMastercard(cardNumber)) {
    return Provider::Mastercard;
  } else if (isAmex(cardNumber)) {
    return Provider::Amex;
  }

  return Provider::Unknown;
}

bool CardValidator::validateLuhn(const QString& cardNumber) {
  if (cardNumber.isEmpty()) return false;

  int sum = 0;
  bool alternate = false;

  for (int i = cardNumber.length() - 1; i >= 0; --i) {
    if (!cardNumber[i].isDigit()) return false;

    int digit = cardNumber[i].digitValue();

    if (alternate) {
      digit *= 2;
      if (digit > 9) {
        digit -= 9;
      }
    }

    sum += digit;
    alternate = !alternate;
  }

  return (sum % 10 == 0);
}

QString CardValidator::formatForDisplay(const QString& cardNumber) {
  QString clean = cardNumber.simplified().remove(u' ');
  if (clean.isEmpty()) return clean;

  QString formatted;
  size_t length = clean.length();

  size_t segmentSize = 4;
  if (isAmex(clean)) {
    // Amex formatting pattern: 4-6-5 digits
    for (size_t i = 0; i < length; ++i) {
      formatted.append(clean[i]);
      if ((i == 3 && length > 4) || (i == 9 && length > 10)) {
        formatted.append(u' ');
      }
    }
    return formatted;
  }

  // Standard formatting pattern: Blocks of 4 digits (Visa, Mastercard,
  // Discover, Maestro)
  for (size_t i = 0; i < length; ++i) {
    if (i > 0 && i % segmentSize == 0) {
      formatted.append(u' ');
    }
    formatted.append(clean[i]);
  }

  return formatted;
}

bool CardValidator::isVisa(const QString& cardNumber) {
  return !cardNumber.isEmpty() && cardNumber.startsWith(u'4');
}

bool CardValidator::isAmex(const QString& cardNumber) {
  if (cardNumber.length() < 2) return false;

  auto prefix2 = QStringView(cardNumber).left(2);
  return prefix2 == u"34" || prefix2 == u"37";
}

bool CardValidator::isMastercard(const QString& cardNumber) {
  size_t length = cardNumber.length();
  if (length < 2) return false;

  auto view = QStringView(cardNumber);
  auto prefix2 = view.left(2);

  // Standard Credit Ranges
  if (prefix2 >= u"51" && prefix2 <= u"55") return true;

  if (length >= 4) {
    auto prefix4 = view.left(4);

    // Modern Credit Series
    if (prefix4 >= u"2221" && prefix4 <= u"2720") return true;

    // Global Maestro Debit
    if (prefix4 == u"5018" || prefix4 == u"5020" || prefix4 == u"5038" ||
        prefix4 == u"5893" || prefix4 == u"6304" || prefix4 == u"6759" ||
        (prefix4 >= u"6761" && prefix4 <= u"6763")) {
      return true;
    }
  }

  if (length >= 6) {
    auto prefix6 = view.left(6);

    // Legacy UK Maestro Debit
    if (prefix6 == u"676770" || prefix6 == u"676774") return true;
  }

  return false;
}
