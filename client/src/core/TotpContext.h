#pragma once
#include <utils/utils.h>

#include <QString>
#include <cstdint>
#include <span>
#include <vector>

class TotpContext {
 public:
  TotpContext();
  ~TotpContext();

  void clearSecret();
  void setSecret(const std::span<uint8_t>& secret);
  bool setSecret(const QString& base32Secret);

  bool hasSecret() const { return !m_secret.empty(); }

  QString generateTotp(
      std::int64_t timePoint = ikea400::utils::unix_seconds()) const;

 private:
  std::vector<uint8_t> m_secret;
};