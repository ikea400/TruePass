#include "validation.h"

#include <cctype>
#include <ctll/fixed_string.hpp>
#include <ctre.hpp>
#include <ctre/wrapper.hpp>
#include <string_view>

using namespace validation;

UsernameValidationResult validation::validateUsername(
    std::string_view username) {
  if (username.length() < kMinUsernameLength) {
    return UsernameValidationResult(UsernameError::TooShort,
                                    "Username is too short. Must be at least " +
                                        std::to_string(kMinUsernameLength) +
                                        " characters.");
  }
  if (username.length() > kMaxUsernameLength) {
    return UsernameValidationResult(
        UsernameError::TooLong, "Username is too long. Must be no more than " +
                                    std::to_string(kMaxUsernameLength) +
                                    " characters.");
  }

  for (char c : username) {
    if (!std::isalnum(static_cast<int>(c)) && c != '_' && c != '-' &&
        c != '.') {
      return UsernameValidationResult(UsernameError::InvalidCharacters,
                                      "Username contains invalid characters.");
    }
  }

  return UsernameValidationResult(UsernameError::None, "");
}

EmailValidationResult validation::validateEmail(std::string_view email) {
  // This pattern checks for: [non-whitespace/@] @ [non-whitespace/@] .
  // [non-whitespace/2+ chars]
  constexpr auto is_email =
      ctll::fixed_string{R"(^[^@\s]+@[^@\s]+\.[^@\s]{2,}$)"};

  constexpr auto matcher = ctre::match<is_email>;

  return matcher(email) ? EmailValidationResult(EmailError::None, "")
                        : EmailValidationResult(EmailError::InvalidFormat,
                                                "Invalid email format.");
}

PasswordValidationResult validation::validatePassword(
    std::string_view password) {
  if (password.length() < kMinPasswordLength)
    return PasswordValidationResult(PasswordError::TooShort,
                                    "Password is too short. Must be at least " +
                                        std::to_string(kMinPasswordLength) +
                                        " characters.");
  if (password.length() > kMaxPasswordLength)
    return PasswordValidationResult(
        PasswordError::TooLong, "Password is too long. Must be no more than " +
                                    std::to_string(kMaxPasswordLength) +
                                    " characters.");

  return PasswordValidationResult(PasswordError::None, "");
}

VaultNameValidationResult validation::validateVaultName(
    std::string_view vaultName) {
  if (vaultName.length() < kMinVaultNameLength) {
    return VaultNameValidationResult(
        VaultNameError::TooShort, "Vault name is too short. Must be at least " +
                                      std::to_string(kMinVaultNameLength) +
                                      " characters.");
  }
  if (vaultName.length() > kMaxVaultNameLength) {
    return VaultNameValidationResult(
        VaultNameError::TooLong,
        "Vault name is too long. Must be no more than " +
            std::to_string(kMaxVaultNameLength) + " characters.");
  }

  for (char c : vaultName) {
    if (!std::isalnum(static_cast<int>(c)) && c != '_' && c != '-' && c != '.') {
      return VaultNameValidationResult(
          VaultNameError::InvalidCharacters,
          "Vault name contains invalid characters.");
    }
  }

  return VaultNameValidationResult(VaultNameError::None, "");
}

VaultDescriptionValidationResult validation::validateVaultDescription(
    std::string_view vaultDescription) {
  if (vaultDescription.length() > kMaxVaultDescriptionLength) {
    return VaultDescriptionValidationResult(
        VaultDescriptionError::TooLong,
        "Vault description is too long. Must be no more than " +
            std::to_string(kMaxVaultDescriptionLength) + " characters.");
  }

  return VaultDescriptionValidationResult(VaultDescriptionError::None, "");
}

VaultItemNameValidationResult validation::validateVaultItemName(
    std::string_view vaultItemName) {
  if (vaultItemName.length() < kMinVaultItemNameLength) {
    return VaultItemNameValidationResult(
        VaultItemNameError::TooShort,
        "Vault item name is too short. Must be at least " +
            std::to_string(kMinVaultItemNameLength) + " characters.");
  }
  if (vaultItemName.length() > kMaxVaultItemNameLength) {
    return VaultItemNameValidationResult(
        VaultItemNameError::TooLong,
        "Vault item name is too long. Must be no more than " +
            std::to_string(kMaxVaultItemNameLength) + " characters.");
  }

  return VaultItemNameValidationResult(VaultItemNameError::None, "");
}

VaultItemNoteValidationResult validation::validateVaultItemNote(
    std::string_view vaultItemNote)
{
  if (vaultItemNote.length() > kMaxVaultItemNoteLength) {
    return VaultItemNoteValidationResult(
        VaultItemNoteError::TooLong,
        "Vault item note is too long. Must be no more than " +
            std::to_string(kMaxVaultItemNoteLength) + " characters.");
  }
  return VaultItemNoteValidationResult(VaultItemNoteError::None, "");
}