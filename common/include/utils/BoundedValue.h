#pragma once
#include <glaze/glaze.hpp>

namespace ikea400 {
template <class T, T MIN, T MAX>
struct BoundedValue {
  static inline constexpr T min_value = MIN;
  static inline constexpr T max_value = MAX;
  T value{};

  inline BoundedValue() noexcept = default;
  inline BoundedValue(T value) noexcept : value(value) {}

  // Allow implicit use like a normal T
  inline operator T() const noexcept { return value; }
  inline BoundedValue& operator=(T v) noexcept {
    value = v;
    return *this;
  }
};
}  // namespace ikea400

namespace glz {
template <class T, T MIN, T MAX>
struct from<JSON, ikea400::BoundedValue<T, MIN, MAX>> {
  template <auto Opts>
  static void op(ikea400::BoundedValue<T, MIN, MAX>& out, is_context auto&& ctx,
                 auto&& it, auto&& end) {
    // First parse the raw value into a temporary T
    T temp{};
    parse<JSON>::op<Opts>(temp, ctx, it, end);
    if (ctx.error != glz::error_code::none) return;

    // Range validation
    if (temp < MIN || temp > MAX) {
      ctx.error = glz::error_code::constraint_violated;
      return;
    }

    // Store the validated value
    out.value = temp;
  }
};

template <class T, T MIN, T MAX>
struct to<JSON, ikea400::BoundedValue<T, MIN, MAX>> {
  template <auto Opts>
  static void op(const ikea400::BoundedValue<T, MIN, MAX>& in,
                 is_context auto&& ctx, auto&& b, auto&& ix) noexcept {
    serialize<JSON>::op<Opts>(in.value, ctx, b, ix);
  }
};
}  // namespace glz