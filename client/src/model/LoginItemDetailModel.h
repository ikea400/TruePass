#pragma once

#include <QString>

class LoginItemDetailModel {
 public:
  const QString& getUsername() const noexcept { return m_username; }
  const QString& getEmail() const noexcept { return m_email; }
  const QString& getPassword() const noexcept { return m_password; }
  const QString& getTotpSecret() const noexcept { return m_totpSecret; }
  const QString& getWebsite() const noexcept { return m_website; }

  void setUsername(const QString& username) { m_username = username; }
  void setEmail(const QString& email) { m_email = email; }
  void setPassword(const QString& password) { m_password = password; }
  void setTotpSecret(const QString& totpSecret) { m_totpSecret = totpSecret; }
  void setWebsite(const QString& website) { m_website = website; }

 private:
  QString m_username;
  QString m_email;
  QString m_password;
  QString m_totpSecret;
  QString m_website;
};