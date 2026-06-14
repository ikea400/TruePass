#pragma once

#include <QString>

class IdentityItemDetailModel {
 public:
  const QString& getFirstName() const noexcept { return m_firstName; }
  const QString& getLastName() const noexcept { return m_lastName; }
  const QString& getAddress() const noexcept { return m_address; }
  const QString& getEmail() const noexcept { return m_email; }
  const QString& getDateOfBirth() const noexcept { return m_dateOfBirth; }
  const QString& getUsername() const noexcept { return m_username; }

  void setFirstName(const QString& firstName) { m_firstName = firstName; }
  void setLastName(const QString& lastName) { m_lastName = lastName; }
  void setAddress(const QString& address) { m_address = address; }
  void setEmail(const QString& email) { m_email = email; }
  void setDateOfBirth(const QString& dateOfBirth) {
    m_dateOfBirth = dateOfBirth;
  }
  void setUsername(const QString& username) { m_username = username; }

 private:
  QString m_firstName;
  QString m_lastName;
  QString m_address;
  QString m_email;
  QString m_dateOfBirth;
  QString m_username;
};