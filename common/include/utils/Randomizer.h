#pragma once

#include <array>
#include <cstdint>
#include <exception>
#include <forward_list>
#include <memory>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "utils.h"

namespace ikea400 {

template <typename Derived>
class Randomizer {
 public:
  void bytes(unsigned char* buffer, size_t length) {
    derived()->bytes(buffer, length);
  }
  uint64_t uniform(uint64_t range) { return derived()->uniform(range); }
  int64_t uniform(int64_t min, int64_t max) {
    return derived()->uniform(min, max);
  }
  uint32_t uniform(uint32_t range) { return derived()->uniform(range); }
  int32_t uniform(int32_t min, int32_t max) {
    return derived()->uniform(min, max);
  }

  template <utils::RangeType T>
  void bytes(T& buffer) {
    bytes(reinterpret_cast<unsigned char*>(buffer.data()),
          buffer.size() * sizeof(typename T::value_type));
  }

  template <std::ranges::random_access_range T>
  void shuffle(T& span) {
    if (span.size() < 2) return;
    for (size_t i = span.size() - 1; i > 0; --i) {
      size_t j = uniform(i + 1);
      std::swap(span[i], span[j]);
    }
  }

  template <std::ranges::random_access_range R>
  constexpr decltype(auto) pick(R&& range) {
    if (std::ranges::empty(range)) {
      throw std::runtime_error("Cannot pick from an empty range");
    }

    using size_type = std::ranges::range_size_t<R>;
    size_type total_size = std::ranges::size(range);

    auto index =
        static_cast<size_type>(uniform(static_cast<uint64_t>(total_size)));
    return range[index];
  }

 private:
  Derived* derived() { return static_cast<Derived*>(this); }
};
}  // namespace ikea400