#include "VaultItem.h"

#include <dto/vault_item_dto.h>
#include <qdebug.h>
#include <utils/ScopedTimer.h>
#include <utils/utils.h>
#include <utils/uuid.h>

#include <cstdint>
#include <glaze/json/read.hpp>
#include <glaze/json/write.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "../dto/CardItemDto.h"
#include "../dto/LoginItemDto.h"
#include "../dto/VaultItemDetailsBase.h"
#include "../dto/VaultItemMetadataDto.h"
#include "../model/LoginItemDetailModel.h"
#include "../model/VaultItemModel.h"
#include "CardValidator.h"
#include "EnvelopeCodec.h"
#include "Vault.h"

using namespace ikea400;

static inline constexpr size_t kMetadataPaddingSize = 128;
static inline constexpr size_t kDataPaddingSize = 512;

template <typename T>
constexpr ikea400::dto::VaultItemType getVaultType();
template <>
constexpr ikea400::dto::VaultItemType getVaultType<LoginItemDetailModel>() {
  return ikea400::dto::VaultItemType::Login;
}
template <>
constexpr ikea400::dto::VaultItemType getVaultType<CardItemDetailModel>() {
  return ikea400::dto::VaultItemType::Card;
}

static std::vector<uint8_t> serializeMetadata(
    const VaultItemModel& model, dto::VaultItemType type,
    std::optional<std::string> custom_icon) {
  dto::VaultItemMetadataDto metadataDto{.name = model.getName().toStdString(),
                                        .version = 1,
                                        .type = type,
                                        .is_deleted = false,
                                        .is_favorite = false,
                                        .custom_icon = std::move(custom_icon)};

  std::vector<uint8_t> buffer;
  auto ec = glz::write_json(metadataDto, buffer);
  if (ec) {
    throw std::runtime_error(std::string("Failed to serialize metadata: ") +
                             std::string(ec.custom_error_message));
  }

  // Pad the buffer to prevent leaking metadata length through ciphertext size
  utils::padJson(buffer, kMetadataPaddingSize);
  return buffer;
}

static dto::VaultItemMetadataDto deserializeMetadata(
    const std::vector<uint8_t>& encryptedMetadata) {
  dto::VaultItemMetadataDto metadataDto;
  auto ec =
      glz::read_json<dto::VaultItemMetadataDto>(metadataDto, encryptedMetadata);
  if (ec) {
    throw std::runtime_error(std::string("Failed to deserialize metadata: ") +
                             std::string(ec.custom_error_message));
  }
  return metadataDto;
}

template <typename T>
static std::vector<uint8_t> serializeData(const T& value) {
  std::vector<uint8_t> buffer;
  auto ec = glz::write_json(value, buffer);
  if (ec) {
    throw std::runtime_error(std::string("Failed to serialize data: ") +
                             std::string(ec.custom_error_message));
  }

  // Pad the buffer to prevent leaking data length through ciphertext size
  utils::padJson(buffer, kDataPaddingSize);
  return buffer;
}

static std::vector<uint8_t> serializeData(
    const VaultItemModel& model, const LoginItemDetailModel& loginDetails) {
  dto::LoginItemDto loginDto{
      .base = VaultItemDetailsBase{.note = model.getNote().toStdString()},
      .username = loginDetails.getUsername().toStdString(),
      .email = loginDetails.getEmail().toStdString(),
      .password = loginDetails.getPassword().toStdString(),
      .totp_secret = loginDetails.getTotpSecret().toStdString(),
      .website = loginDetails.getWebsite().toStdString(),
  };
  return serializeData(loginDto);
}

static std::vector<uint8_t> serializeData(
    const VaultItemModel& model, const CardItemDetailModel& cardDetails) {
  dto::CardItemDto cardDto{
      .base = VaultItemDetailsBase{.note = model.getNote().toStdString()},
      .cardholder_name = cardDetails.getCardholderName().toStdString(),
      .card_number = cardDetails.getCardNumber().toStdString(),
      .cvv = cardDetails.getCvv().toStdString(),
      .billing_address = cardDetails.getBillingAddress().toStdString(),
      .expiry_month = cardDetails.getExpirationMonth(),
      .expiry_year = cardDetails.getExpirationYear(),
  };

  return serializeData(cardDto);
}

template <typename T>
T deserializeData(const std::vector<uint8_t>& decryptedData) {
  T data;
  auto ec = glz::read_json<T>(data, decryptedData);
  if (ec) {
    throw std::runtime_error(std::string("Failed to deserialize data: ") +
                             std::string(ec.custom_error_message));
  }
  return data;
}

static std::pair<LoginItemDetailModel, VaultItemDetailsBase>
deserializeLoginData(const std::vector<uint8_t>& decryptedData) {
  dto::LoginItemDto loginDto =
      deserializeData<dto::LoginItemDto>(decryptedData);
  LoginItemDetailModel details;
  details.setUsername(QString::fromStdString(loginDto.username));
  details.setEmail(QString::fromStdString(loginDto.email));
  details.setPassword(QString::fromStdString(loginDto.password));
  details.setTotpSecret(QString::fromStdString(loginDto.totp_secret));
  details.setWebsite(QString::fromStdString(loginDto.website));
  return std::make_pair(details, loginDto.base);
}

static std::pair<CardItemDetailModel, VaultItemDetailsBase> deserializeCardData(
    const std::vector<uint8_t>& decryptedData) {
  dto::CardItemDto cardDto = deserializeData<dto::CardItemDto>(decryptedData);
  CardItemDetailModel details;
  details.setCardholderName(QString::fromStdString(cardDto.cardholder_name));
  details.setCardNumber(QString::fromStdString(cardDto.card_number));
  details.setCvv(QString::fromStdString(cardDto.cvv));
  details.setBillingAddress(QString::fromStdString(cardDto.billing_address));
  details.setExpirationDate(cardDto.expiry_month, cardDto.expiry_year);
  return std::make_pair(details, cardDto.base);
}

static std::optional<std::string> getCustomIconFromDetails(
    const LoginItemDetailModel& loginDetails) {
  // For login items, we can use the website to determine a custom icon
  std::string website = loginDetails.getWebsite().toStdString();
  if (website.empty()) {
    return std::nullopt;
  }

  std::string domain = utils::extractDomain(website);
  if (domain.empty() || !website.contains('.')) {
    return std::nullopt;
  }

  return domain;
}

static std::optional<std::string> getCustomIconFromDetails(
    const CardItemDetailModel& cardDetails) {
  std::string brandName;

  auto provider = CardValidator::detectProvider(cardDetails.getCardNumber());
  switch (provider) {
    case CardValidator::Provider::Visa:
      brandName = "visa";
      break;
    case CardValidator::Provider::Mastercard:
      brandName = "mastercard";
      break;
    case CardValidator::Provider::Amex:
      brandName = "amex";
      break;
    default:
      break;
  }

  if (brandName.empty()) return std::nullopt;

  return brandName;
}

static std::optional<std::string> getCustomIconFromDetails(
    const auto& /*details*/) {
  // For other item types, we currently don't have a way to determine a custom
  // icon
  return std::nullopt;
}

VaultItem VaultItem::createFromModel(const VaultItemModel& model,
                                     const Vault& vault) {
  if (!vault.isDecrypted())
    throw std::runtime_error("Vault must be decrypted to create VaultItem");

  std::string customIcon;
  ikea400::dto::VaultItemType type;

  const auto visitor = utils::overloads{[&](const auto& details) {
    using DetailsType = std::decay_t<decltype(details)>;

    type = getVaultType<DetailsType>();
    customIcon = getCustomIconFromDetails(details).value_or({});

    return std::make_pair(serializeMetadata(model, type, customIcon),
                          serializeData(model, details));
  }};

  const auto& [serializedMetadata, serializedData] =
      std::visit(visitor, model.getDetails());

  VaultItem item;
  item.m_updatedAt = item.m_createdAt = utils::unix_seconds();
  item.m_id = uuid::fromString(model.getId().toStdString());
  item.m_vaultId = vault.getId();
  item.m_name = model.getName().toStdString();
  item.m_customIcon = customIcon;
  item.m_type = type;
  item.m_isDeleted = item.m_isFavorite = false;
  item.m_encrypted_data =
      EnvelopeCodec::encode(serializedData, vault.getVaultKey(),
                            kVaultItemDataBindingKey, item.m_id.bytes());
  item.m_encrypted_metadata =
      EnvelopeCodec::encode(serializedMetadata, vault.getVaultKey(),
                            kVaultItemMetadataBindingKey, item.m_id.bytes());
  item.m_hasData = true;

  if (item.m_encrypted_metadata.empty() || item.m_encrypted_data.empty()) {
    throw std::runtime_error("Failed to encrypt VaultItem data");
  }

  return item;
}

VaultItem VaultItem::loadFromSummary(
    const ikea400::dto::VaultItemSummary& summary, const Vault& vault) {
  if (!vault.isDecrypted())
    throw std::runtime_error("Vault must be decrypted to load VaultItem");

  std::vector<uint8_t> decryptedMetadata =
      EnvelopeCodec::decode(summary.protectedMetaData.data, vault.getVaultKey(),
                            kVaultItemMetadataBindingKey, summary.id.bytes());

  if (decryptedMetadata.empty()) {
    throw std::runtime_error("Failed to decrypt VaultItem metadata");
  }

  ikea400::dto::VaultItemMetadataDto metadataDto =
      deserializeMetadata(decryptedMetadata);

  VaultItem item;
  item.m_encrypted_metadata = summary.protectedMetaData.data;
  item.m_id = summary.id;
  item.m_vaultId = summary.vaultId;
  item.m_createdAt = summary.createdAt;
  item.m_name = metadataDto.name;
  item.m_isDeleted = metadataDto.is_deleted;
  item.m_isFavorite = metadataDto.is_favorite;
  item.m_type = metadataDto.type;
  item.m_customIcon = metadataDto.custom_icon.value_or({});
  return item;
}

VaultItemModel VaultItem::toModel(const Vault& vault) const {
  ScopedTimer timer("VaultItem::toModel");
  if (!vault.isDecrypted())
    throw std::runtime_error(
        "Vault must be decrypted to convert VaultItem to model");
  if (!hasData()) {
    throw std::runtime_error(
        "VaultItem has no data. Cannot convert to model without data.");
  }

  std::vector<uint8_t> decryptedData =
      EnvelopeCodec::decode(m_encrypted_data, vault.getVaultKey(),
                            kVaultItemDataBindingKey, m_id.bytes());

  if (decryptedData.empty()) {
    throw std::runtime_error("Failed to decrypt VaultItem data");
  }

  VaultItemModel model;
  model.setName(QString::fromStdString(m_name));
  model.setId(QString::fromStdString(m_id.toString()));
  model.setIsFavorite(m_isFavorite);

  switch (m_type) {
    case dto::VaultItemType::Login: {
      auto [loginDetails, baseDetails] = deserializeLoginData(decryptedData);
      model.set(std::move(loginDetails));
      model.setNote(QString::fromStdString(baseDetails.note));
    } break;
    case dto::VaultItemType::Card: {
      auto [cardDetails, baseDetails] = deserializeCardData(decryptedData);
      model.set(std::move(cardDetails));
      model.setNote(QString::fromStdString(baseDetails.note));
    } break;
    default:
      throw std::runtime_error("Unsupported VaultItem type: " +
                               std::to_string(static_cast<uint8_t>(m_type)));
      break;
  }

  return model;
}
