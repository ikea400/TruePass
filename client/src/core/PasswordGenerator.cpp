#include "PasswordGenerator.h"

#include <utils/CryptoRandomizer.h>
#include <utils/SecretRandomizer.h>
#include <utils/utils.h>

#include <QStringList>

#include "wordlist.h"

using namespace ikea400;

static SecretRandomizer secretRandomizer;
static CryptoRandomizer cryptoRandomizer;

QString PasswordGenerator::generatePassword(int length, bool includeUppercase,
                                            bool includeLowercase,
                                            bool includeDigits,
                                            bool includeSpecial,
                                            bool ambiguous) {
  QString charset = makeCharacterSet(includeUppercase, includeLowercase,
                                     includeDigits, includeSpecial, ambiguous);

  if (charset.isEmpty() || length <= 0) {
    return QString();
  }

  cryptoRandomizer.shuffle(charset);

  const uint32_t charsetSize = static_cast<uint32_t>(charset.length());

  QString password(length, Qt::Uninitialized);
  for (int i = 0; i < password.length(); ++i) {
    password[i] = charset[secretRandomizer.uniform(charsetSize)];
  }

  return password;
}

QString PasswordGenerator::generatePassphrase(int numWords, bool uppercase,
                                              bool digits, QChar seperator) {
  static_assert(kEffLongWordlist.size() <= std::numeric_limits<uint32_t>::max(),
                "Wordlist size exceeds uint32_t max value");

  int luckyNumber = -1;
  if (digits) {
    luckyNumber = static_cast<int>(
        secretRandomizer.uniform(static_cast<uint32_t>(numWords)));
  }

  const uint32_t wordListSize = static_cast<uint32_t>(kEffLongWordlist.size());

  QStringList phrase;
  for (int i = 0; i < numWords; ++i) {
    uint32_t index = secretRandomizer.uniform(wordListSize);

    // Using .at for additional bounds checking, even though
    // secureRandomUniform should never return an out-of-bounds index.
    std::string_view wordView = kEffLongWordlist.at(index);

    QString word =
        QString::fromUtf8(wordView.data(), static_cast<int>(wordView.size()));

    if (uppercase && !word.isEmpty()) {
      word[0] = word[0].toUpper();
    }

    if (i == luckyNumber) {
      word += QString::number(secretRandomizer.uniform(10u));
    }

    phrase.append(word);
  }

  return phrase.join(seperator);
}

QString PasswordGenerator::makeCharacterSet(bool includeUppercase,
                                            bool includeLowercase,
                                            bool includeDigits,
                                            bool includeSpecial,
                                            bool ambiguous) {
  QString charset;

  charset.reserve(kUppercaseChars.size() + kLowercaseChars.size() +
                  kDigitChars.size() + kSpecialChars.size());

  auto addChars = [&](bool condition, const QString& normal,
                      const QString& ambig = QString()) {
    if (!condition) return;

    if (ambig.isEmpty()) {
      charset += normal;
    } else {
      charset += (ambiguous ? ambig : normal);
    }
  };

  addChars(includeUppercase, kUppercaseChars, kUppercaseCharsAmbiguous);
  addChars(includeLowercase, kLowercaseChars, kLowercaseCharsAmbiguous);
  addChars(includeDigits, kDigitChars, kDigitCharsAmbiguous);
  addChars(includeSpecial, kSpecialChars);

  return charset;
}
