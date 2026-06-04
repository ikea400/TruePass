#pragma once

#include <optional>
#include <string>

namespace ikea400 {

template <class T>
struct ResponseMessage {
  std::optional<T> data;
  std::optional<std::string> error;
  std::optional<uint64_t> error_code;
  int32_t http_status;
  bool success;
};

template <>
struct ResponseMessage<void> {
  std::optional<std::string> error;
  std::optional<uint64_t> error_code;
  int32_t http_status;
  bool success;
};

}  // namespace ikea400