#pragma once
#include <memory>
#include <string>

namespace ikea400 {
using SharedStrings = std::shared_ptr<const std::string>;
// Alias for a function that creates a shared_string
template <typename... Args>
SharedStrings makeSharedString(Args&&... args) {
  return std::make_shared<SharedStrings::element_type>(
      std::forward<Args>(args)...);
}
}  // namespace ikea400
