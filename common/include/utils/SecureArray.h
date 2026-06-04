#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <span>
#include <type_traits>
#include <utility>

#include "utils.h"

namespace ikea400 {

template <typename T, std::size_t N>
  requires std::is_trivially_copyable_v<T>
class SecureArray {
 public:
  using value_type = T;
  using const_value_type = const T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using pointer = value_type*;
  using const_pointer = const value_type*;

  using iterator = pointer;
  using const_iterator = const_pointer;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // ---- Constructors ----
  constexpr SecureArray() noexcept = default;

  SecureArray(const SecureArray&) = default;
  SecureArray& operator=(const SecureArray&) = default;

  explicit SecureArray(const std::array<value_type, N>& arr) noexcept {
    std::copy_n(arr.data(), N, m_data.data());
  }

  // Move constructor
  explicit constexpr SecureArray(SecureArray&& other) noexcept {
    std::copy_n(other.m_data.data(), N, m_data.data());
    utils::secureErase(other.m_data);
  }

  // Move assignment
  constexpr SecureArray& operator=(SecureArray&& other) noexcept {
    if (this != &other) {
      utils::secureErase(m_data);
      std::copy_n(other.m_data.data(), N, m_data.data());
      utils::secureErase(other.m_data);
    }
    return *this;
  }

  // Destructor securely wipes memory
  ~SecureArray() noexcept { utils::secureErase(m_data); }

  // ---- Capacity ----
  [[nodiscard]] constexpr size_type size() const noexcept { return N; }
  [[nodiscard]] constexpr bool empty() const noexcept { return N == 0; }

  // ---- Fill ----
  constexpr void fill(value_type value) noexcept { m_data.fill(value); }

  // ---- Clear ----
  void clear() noexcept { utils::secureErase(m_data); }

  // ---- Element access ----
  [[nodiscard]] constexpr value_type& operator[](size_type index) noexcept {
    return m_data[index];
  }
  [[nodiscard]] constexpr const value_type& operator[](
      size_type index) const noexcept {
    return m_data[index];
  }

  // ---- Data access ----
  [[nodiscard]] constexpr pointer data() noexcept { return m_data.data(); }
  [[nodiscard]] constexpr const_pointer data() const noexcept {
    return m_data.data();
  }

  // ---- Iterators ----
  [[nodiscard]] constexpr iterator begin() noexcept { return data(); }
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return data();
  }
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return data();
  }

  [[nodiscard]] constexpr iterator end() noexcept { return data() + N; }
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return data() + N;
  }
  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return data() + N;
  }

  [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }
  [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  [[nodiscard]] constexpr reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }
  [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }
  [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

  // ---- Span conversion ----
  [[nodiscard]] constexpr std::span<T> asSpan() noexcept {
    return std::span<T>(data(), size());
  }

  [[nodiscard]] constexpr std::span<const T> asSpan() const noexcept {
    return std::span<const T>(data(), size());
  }

  [[nodiscard]] constexpr std::span<const T, N> asArraySpan() const noexcept {
    return std::span<const T, N>(data(), size());
  }

  constexpr operator std::span<T>() noexcept { return {data(), size()}; }
  constexpr operator std::span<const T>() const noexcept {
    return {data(), size()};
  }

 private:
  std::array<value_type, N> m_data{};
};

}  // namespace ikea400
