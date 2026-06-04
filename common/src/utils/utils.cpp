#include "../../include/utils/utils.h"

#include <openssl/crypto.h>
#include <openssl/rand.h>

#include <bit>
#include <cstdint>
#include <ctre.hpp>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using namespace ikea400;

void utils::secureErase(void* ptr, size_t len) noexcept {
  OPENSSL_cleanse(ptr, len);
}

/**
 * @brief perform a Truncated Lempel-Ziv Complexity Heuristic to estimate the
 * complexity of a byte sequence.
 */
double utils::estimateEntropy(const uint8_t* data, size_t length) {
  if (length < 2) return 0.0;

  // Calcul de la taille du bitset
  constexpr size_t bit_resolution =
      16;  // Nombre de bits à considérer pour chaque paire d'octets
  constexpr size_t num_combinations = 1ULL << bit_resolution;
  constexpr size_t num_words = num_combinations / 64;

  // Calcul de la taille du bitset
  uint64_t bitset[num_words] = {
      0};  // Alloué sur la pile, taille fixe selon le template

  const uint8_t* ptr = data;
  const size_t n = length - 1;

  // Masque pour tronquer les bits si Resolution < 16
  constexpr uint16_t mask = static_cast<uint16_t>(num_combinations - 1);

  for (size_t i = 0; i < n; ++i) {
    // On combine les deux octets et on applique le masque
    uint16_t pair = ((static_cast<uint16_t>(ptr[i]) << 8) | ptr[i + 1]) & mask;
    bitset[pair >> 6] |= (1ULL << (pair & 63));
  }

  size_t unique_count = 0;
  for (size_t i = 0; i < num_words; ++i) {
    unique_count += std::popcount(bitset[i]);
  }

  return static_cast<double>(unique_count) / n;
}

void utils::padJson(std::vector<uint8_t>& jsonData, size_t blockSize) {
  size_t currentLen = jsonData.size();
  size_t delta = (currentLen % blockSize);

  if (delta > 0) {
    size_t paddingNeeded = blockSize - delta;
    jsonData.insert(jsonData.end(), paddingNeeded, static_cast<uint8_t>(' '));
  }
}

std::string utils::extractDomain(std::string_view url) {
  if (auto match =
          ctre::search<"^(?:https?://)?(?:www\\.)?([^:/\\s?#]+)">(url)) {
    // Return the first capture group (the domain)
    return match.get<1>().to_string();
  }
  return {};
}