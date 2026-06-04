#pragma once
#include <glaze/glaze.hpp>
#include <span>
#include <vector>

#include "BinEncoding.h"

namespace ikea400 {
template <size_t MIN, size_t MAX>
struct BinVector {
  std::vector<uint8_t> data;

  std::vector<uint8_t>& value() noexcept { return data; }
  const std::vector<uint8_t>& value() const noexcept { return data; }

  // ----------------- Constructors -----------------
  constexpr BinVector() : data{} {}

  constexpr BinVector(const std::vector<uint8_t>& arr) : data(arr) {}
  constexpr BinVector(std::vector<uint8_t>&& arr) : data(std::move(arr)) {}
  constexpr BinVector(std::span<const uint8_t> arr)
      : data(arr.begin(), arr.end()) {}
  constexpr BinVector(std::span<uint8_t> arr) : data(arr.begin(), arr.end()) {}

  constexpr BinVector(const BinVector&) = default;
  constexpr BinVector(BinVector&&) noexcept = default;

  BinVector& operator=(const BinVector&) = default;
  BinVector& operator=(BinVector&&) noexcept = default;
  BinVector& operator=(std::span<const uint8_t> arr) {
    data.assign(arr.begin(), arr.end());
    return *this;
  }
  BinVector& operator=(std::span<uint8_t> arr) {
    data.assign(arr.begin(), arr.end());
    return *this;
  }
};
}  // namespace ikea400

namespace glz {

template <size_t MIN, size_t MAX>
struct from<JSON, ikea400::BinVector<MIN, MAX>> {
  template <auto Opts>
  static void op(ikea400::BinVector<MIN, MAX>& value, is_context auto&& ctx,
                 auto&& it, auto&& end) {
    // Parse the JSON string first
    std::string formated;
    parse<JSON>::op<Opts>(formated, ctx, it, end);

    // Stop if parsing already failed
    if (ctx.error != glz::error_code::none) return;

    // Decode format(hex, b64) => bytes
    try {
      value.data = ikea400::bin::decodeFormat(formated);
    } catch (...) {
      ctx.error = glz::error_code::constraint_violated;
      return;
    }

    // Validate byte length constraints
    if (value.data.size() < MIN || value.data.size() > MAX) {
      ctx.error = glz::error_code::constraint_violated;
      return;
    }
  }
};

template <size_t MIN, size_t MAX>
struct to<JSON, ikea400::BinVector<MIN, MAX>> {
  template <auto Opts>
  static void op(const ikea400::BinVector<MIN, MAX>& value,
                 is_context auto&& ctx, auto&& b, auto&& ix) noexcept {
    std::string formated = ikea400::bin::b64::format(value.data);
    serialize<JSON>::op<Opts>(formated, ctx, b, ix);
  }
};
}  // namespace glz