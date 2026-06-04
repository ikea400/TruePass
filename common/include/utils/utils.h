#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <limits>
#include <ranges>
#include <vector>

namespace ikea400::utils {

template <auto fn>
using DeleterFromFn = std::integral_constant<decltype(fn), fn>;

template <typename T>
concept RangeType = requires(T& t) {
  // 1. Must have the nested type alias
  typename T::value_type;

  // 2. Must have .size() returning an integral type
  { t.size() } -> std::integral;

  // 3. Must have .data() returning a pointer to the value_type
  // We handle const/non-const by checking against the range_reference_t
  {
    t.data()
  } -> std::same_as<std::add_pointer_t<
      std::remove_reference_t<std::ranges::range_reference_t<T>>>>;
};

// 1. The generic trait
template <typename T, template <typename...> class Template>
struct is_specialization_of : std::false_type {};

// 2. The specialization
template <template <typename...> class Template, typename... Args>
struct is_specialization_of<Template<Args...>, Template> : std::true_type {};

// 3. Helper variable template
template <typename T, template <typename...> class Template>
inline constexpr bool is_specialization_of_v =
    is_specialization_of<T, Template>::value;

// 4. Clean definition of is_duration_v
template <typename T>
inline constexpr bool is_duration_v =
    is_specialization_of_v<T, std::chrono::duration>;

// helper type for the visitor
template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

template <std::integral T>
[[nodiscard]] constexpr T native_to_big(T value) noexcept {
  if constexpr (std::endian::native == std::endian::little) {
    return std::byteswap(value);
  } else {
    return value;
  }
}

template <std::integral T>
[[nodiscard]] constexpr T big_to_native(T value) noexcept {
  if constexpr (std::endian::native == std::endian::little) {
    return std::byteswap(value);
  } else {
    return value;
  }
}

inline bool ichar_equals(char a, char b) {
  return std::tolower(static_cast<unsigned char>(a)) ==
         std::tolower(static_cast<unsigned char>(b));
}

inline bool iequals(std::string_view lhs, std::string_view rhs) {
  return std::ranges::equal(lhs, rhs, ichar_equals);
}

inline std::int64_t unix_seconds() {
  using namespace std::chrono;
  return time_point_cast<seconds>(system_clock::now())
      .time_since_epoch()
      .count();
}

void secureErase(void* ptr, size_t len) noexcept;

template <RangeType T>
inline void secureErase(T& data) noexcept {
  static_assert(std::is_trivially_copyable_v<typename T::value_type>,
                "secureErase requires trivially copyable element types.");
  secureErase(reinterpret_cast<void*>(data.data()),
              data.size() * sizeof(typename T::value_type));
}

double estimateEntropy(const uint8_t* data, size_t length);

template <RangeType T>
inline double estimateEntropy(const T& data) {
  return estimateEntropy(reinterpret_cast<const uint8_t*>(data.data()),
                         data.size() * sizeof(typename T::value_type));
}

template <RangeType T>
bool isEntropySufficient(const T& data, double threshold = 0.8) {
  return estimateEntropy(data) >= threshold;
}

void padJson(std::vector<uint8_t>& jsonData, size_t blockSize = 512);

std::string extractDomain(std::string_view url);

}  // namespace ikea400::utils