#pragma once

#include "VaultItemDetailsBase.h"
#include <utils/BoundedValue.h>

namespace ikea400::dto {
struct CardItemDto {
  VaultItemDetailsBase base;
  std::string cardholder_name;
  std::string card_number;
  std::string cvv;
  std::string billing_address;
  BoundedValue<int, 0, 12> expiry_month;  // Allow 0 for "no expiration month"
  BoundedValue<int, 0, 99> expiry_year;  // Allow 0 for "no expiration year"
};
}  // namespace ikea400::dto