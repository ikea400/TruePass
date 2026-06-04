#include "TotpContext.h"

#include <crypto/hash_algo.h>
#include <crypto/hmac.h>
#include <qchar.h>
#include <qlogging.h>
#include <qdebug.h>
#include <qstring.h>
#include <utils/utils.h>

#include <array>
#include <cstdint>
#include <exception>
#include <span>

#include "Base32Decoder.h"

TotpContext::TotpContext() {}

TotpContext::~TotpContext() {}

void TotpContext::clearSecret() {
  if (!m_secret.empty()) {
    ikea400::utils::secureErase(m_secret);
    m_secret.clear();
  }
}

void TotpContext::setSecret(const std::span<uint8_t>& secret) {
  clearSecret();
  m_secret.assign_range(secret);
}

bool TotpContext::setSecret(const QString& base32Secret) {
  try {
    clearSecret();
    m_secret = Base32Decoder::decode(base32Secret);
    return true;

  } catch (const std::exception& e) {
    qWarning() << "Failed to decode Base32 secret:" << e.what()
               << "Input was:" << base32Secret;
    return false;
  }
}

QString TotpContext::generateTotp(std::int64_t timePoint) const {
  if (m_secret.empty()) return {};

  constexpr std::int64_t TOTP_PERIOD = 30;  // seconds
  std::int64_t timeStep = timePoint / TOTP_PERIOD;

  using namespace ikea400::crypto;

  std::array<std::int64_t, 1> timeStepArray{
      ikea400::utils::native_to_big(timeStep)};

  std::array<uint8_t, SHA1_HASH_SIZE> hashOutput;
  size_t size =
      hmac::derive<false>(HashAlgo::Sha1, m_secret, timeStepArray, hashOutput);
  if (size != SHA1_HASH_SIZE) return {};

  // 1. Dynamic Truncation: Determine offset from the last nibble
  const size_t offset = hashOutput[SHA1_HASH_SIZE - 1] & 0x0F;

  // 2. Extract 4 bytes starting at offset, masking the MSB (to avoid signedness
  // issues) This effectively treats the 4 bytes as a big-endian 31-bit integer.
  const uint32_t binary =
      (static_cast<uint32_t>(hashOutput[offset] & 0x7f) << 24) |
      (static_cast<uint32_t>(hashOutput[offset + 1] & 0xff) << 16) |
      (static_cast<uint32_t>(hashOutput[offset + 2] & 0xff) << 8) |
      (static_cast<uint32_t>(hashOutput[offset + 3] & 0xff));

  // 3. Modulo 10^6 to get a 6-digit OTP
  const uint32_t otp = binary % 1000000;

  // 4. Format with zero-padding to 6 digits
  return QString("%1").arg(otp, 6, 10, QLatin1Char('0'));
}
