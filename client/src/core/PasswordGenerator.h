#pragma once
#include <QChar>
#include <QString>

class PasswordGenerator {
 public:
  static QString generatePassword(int length, bool includeUppercase,
                                  bool includeLowercase, bool includeDigits,
                                  bool includeSpecial, bool ambiguous);
  static QString generatePassphrase(int numWords, bool uppercase, bool digits,
                                    QChar seperator);

  static QString makeCharacterSet(bool includeUppercase, bool includeLowercase,
                                  bool includeDigits, bool includeSpecial,
                                  bool ambiguous);

  static inline const QString kUppercaseChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static inline const QString kUppercaseCharsAmbiguous = "ABCDEFGHJKLMNPQRSTUVWXYZ";
  static inline const QString kLowercaseChars = "abcdefghijklmnopqrstuvwxyz";
  static inline const QString kLowercaseCharsAmbiguous = "abcdefghijkmnopqrstuvwxyz";
  static inline const QString kDigitChars = "0123456789";
  static inline const QString kDigitCharsAmbiguous = "23456789";
  static inline const QString kSpecialChars = "!@#$%^&*()-+";
};