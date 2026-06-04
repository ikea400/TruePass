#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <glaze/glaze.hpp>
#include <span>

#include "BinEncoding.h"

namespace ikea400 {
template <size_t N>
struct BinArray {
  std::array<uint8_t, N> data;

  std::array<uint8_t, N>& value() noexcept { return data; }
  const std::array<uint8_t, N>& value() const noexcept { return data; }

  // ----------------- Constructors -----------------
  constexpr BinArray() : data{} {}

  constexpr BinArray(const std::array<uint8_t, N>& arr) : data(arr) {}
  constexpr BinArray(std::array<uint8_t, N>&& arr) : data(std::move(arr)) {}
  constexpr BinArray(std::span<const uint8_t, N> arr) : data{} {
    std::copy(arr.begin(), arr.end(), data.begin());
  }
  BinArray(std::span<const uint8_t> arr) {
    if (arr.size() != data.size()) {
      throw std::invalid_argument(
          "Input span size does not match BinArray size");
    }
    std::copy(arr.begin(), arr.end(), data.begin());
  }

  constexpr BinArray(const BinArray&) = default;
  constexpr BinArray(BinArray&&) noexcept = default;

  BinArray& operator=(const BinArray&) = default;
  BinArray& operator=(BinArray&&) noexcept = default;

  BinArray& operator=(std::span<const uint8_t> arr) {
    if (arr.size() != data.size()) {
      throw std::invalid_argument(
          "Input span size does not match BinArray size");
    }
    std::copy(arr.begin(), arr.end(), data.begin());
    return *this;
  }
};

}  // namespace ikea400

namespace glz {

template <size_t N>
struct from<JSON, ikea400::BinArray<N>> {
  template <auto Opts>
  static void op(ikea400::BinArray<N>& value, is_context auto&& ctx, auto&& it,
                 auto&& end) {
    // Parse the JSON string first
    std::string formated;
    parse<JSON>::op<Opts>(formated, ctx, it, end);

    // Stop if parsing already failed
    if (ctx.error != glz::error_code::none) return;

    std::vector<uint8_t> decoded;
    // Decode format(hex, b64) => bytes
    try {
      decoded = ikea400::bin::decodeFormat(formated);
    } catch (...) {
      ctx.error = glz::error_code::constraint_violated;
      return;
    }

    if (decoded.size() != N) {
      ctx.error = glz::error_code::constraint_violated;
      return;
    }

    std::copy(decoded.begin(), decoded.end(), value.data.begin());
  }
};

template <size_t N>
struct to<JSON, ikea400::BinArray<N>> {
  template <auto Opts>
  static void op(const ikea400::BinArray<N>& value, is_context auto&& ctx,
                 auto&& b, auto&& ix) noexcept {
    std::string formated = ikea400::bin::b64::format(value.data);
    serialize<JSON>::op<Opts>(formated, ctx, b, ix);
  }
};
}  // namespace glz