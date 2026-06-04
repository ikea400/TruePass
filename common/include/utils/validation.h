#pragma once
#include <iostream>
#include <string>
#include <string_view>

namespace validation {

enum class UsernameError { None, TooShort, TooLong, InvalidCharacters };
enum class EmailError { None, InvalidFormat };
enum class PasswordError { None, TooShort, TooLong, Mismatch };
enum class VaultNameError { None, TooShort, TooLong, InvalidCharacters };
enum class VaultDescriptionError { None, TooLong };
enum class VaultItemNameError { None, TooShort, TooLong };
enum class VaultItemNoteError { None, TooLong };

template <typename T>
class ValidationResult {
 public:
  ValidationResult(T r, std::string msg)
      : m_result(r), m_errorMessage(std::move(msg)) {
    if (!isValid() && m_errorMessage.empty()) {
      m_errorMessage = "Unknown error";
    }
  }
  bool isValid() const { return m_result == T::None; }
  const std::string& error() const { return m_errorMessage; }
  const T result() const { return m_result; }

 private:
  std::string m_errorMessage;
  T m_result;
};

using UsernameValidationResult = ValidationResult<UsernameError>;
using EmailValidationResult = ValidationResult<EmailError>;
using PasswordValidationResult = ValidationResult<PasswordError>;
using VaultNameValidationResult = ValidationResult<VaultNameError>;
using VaultDescriptionValidationResult =
    ValidationResult<VaultDescriptionError>;
using VaultItemNameValidationResult = ValidationResult<VaultItemNameError>;
using VaultItemNoteValidationResult = ValidationResult<VaultItemNoteError>;

UsernameValidationResult validateUsername(std::string_view username);
EmailValidationResult validateEmail(std::string_view email);
PasswordValidationResult validatePassword(std::string_view password);
VaultNameValidationResult validateVaultName(std::string_view vaultName);
VaultDescriptionValidationResult validateVaultDescription(
    std::string_view vaultDescription);
VaultItemNameValidationResult validateVaultItemName(
    std::string_view vaultItemName);
VaultItemNoteValidationResult validateVaultItemNote(
    std::string_view vaultItemNote);

constexpr const size_t kMinUsernameLength = 3;
constexpr const size_t kMaxUsernameLength = 32;
constexpr const size_t kMinPasswordLength = 8;
constexpr const size_t kMaxPasswordLength = 100;
constexpr const size_t kMinVaultNameLength = 3;
constexpr const size_t kMaxVaultNameLength = 30;
constexpr const size_t kMaxVaultDescriptionLength = 200;
constexpr const size_t kMinVaultItemNameLength = 3;
constexpr const size_t kMaxVaultItemNameLength = 50;
constexpr const size_t kMaxVaultItemNoteLength = 1024;

}  // namespace validation
