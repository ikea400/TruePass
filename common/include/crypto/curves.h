#pragma once

#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "hash_algo.h"

namespace ikea400::crypto::curves {

class CryptoError final : public std::runtime_error {
 public:
  explicit CryptoError(const std::string& message)
      : std::runtime_error(message) {}
};

enum class Curve { P256, P384, P521, Secp256k1, Ed25519, Ed448, X25519, X448 };

std::string_view toString(Curve curve) noexcept;

enum class SignatureAlgorithmKind { Ecdsa, EdDsa };

class SignatureAlgorithm final {
 public:
  static SignatureAlgorithm ecdsa(HashAlgo hash);
  static SignatureAlgorithm eddsa();

  [[nodiscard]] SignatureAlgorithmKind kind() const noexcept { return m_kind; }
  [[nodiscard]] HashAlgo hash() const noexcept { return m_hash; }

 private:
  SignatureAlgorithm(SignatureAlgorithmKind kind, HashAlgo hash) noexcept
      : m_kind(kind), m_hash(hash) {}

  SignatureAlgorithmKind m_kind;
  HashAlgo m_hash;
};

enum class KeyAgreementAlgorithm { Ecdh, Xdh };

class Signature final {
 public:
  Signature() = default;
  explicit Signature(std::vector<uint8_t> data) : m_data(std::move(data)) {}
  explicit Signature(std::span<const uint8_t> data)
      : m_data(data.begin(), data.end()) {}

  [[nodiscard]] std::span<const uint8_t> bytes() const noexcept {
    return {m_data.data(), m_data.size()};
  }
  [[nodiscard]] size_t size() const noexcept { return m_data.size(); }
  [[nodiscard]] bool empty() const noexcept { return m_data.empty(); }

 private:
  std::vector<uint8_t> m_data;
};

class Secret final {
 public:
  Secret() = default;
  explicit Secret(std::vector<uint8_t> data) : m_data(std::move(data)) {}
  Secret(const Secret&) = delete;
  Secret& operator=(const Secret&) = delete;

  Secret(Secret&& other) noexcept;
  Secret& operator=(Secret&& other) noexcept;
  ~Secret() noexcept;

  [[nodiscard]] std::span<const uint8_t> bytes() const noexcept {
    return {m_data.data(), m_data.size()};
  }
  [[nodiscard]] size_t size() const noexcept { return m_data.size(); }
  [[nodiscard]] bool empty() const noexcept { return m_data.empty(); }

 private:
  void clear() noexcept;

  std::vector<uint8_t> m_data;
};

class PublicKey final {
 public:
  PublicKey() = default;

  static PublicKey fromDer(std::span<const uint8_t> der);
  static PublicKey fromPem(std::string_view pem);

  [[nodiscard]] std::vector<uint8_t> toDer() const;
  [[nodiscard]] std::string toPem() const;

  [[nodiscard]] Curve curve() const noexcept { return m_curve; }
  [[nodiscard]] std::span<const uint8_t> der() const noexcept {
    return {m_der.data(), m_der.size()};
  }
  [[nodiscard]] bool empty() const noexcept { return m_der.empty(); }

 private:
  friend class PrivateKey;
  friend class KeyPair;
  explicit PublicKey(Curve curve, std::vector<uint8_t> der)
      : m_curve(curve), m_der(std::move(der)) {}

  Curve m_curve{Curve::P256};
  std::vector<uint8_t> m_der;
};

class PrivateKey final {
 public:
  PrivateKey() = default;
  PrivateKey(const PrivateKey&) = delete;
  PrivateKey& operator=(const PrivateKey&) = delete;

  PrivateKey(PrivateKey&& other) noexcept;
  PrivateKey& operator=(PrivateKey&& other) noexcept;
  ~PrivateKey() noexcept;

  static PrivateKey fromDer(std::span<const uint8_t> der);
  static PrivateKey fromPem(std::string_view pem);

  [[nodiscard]] std::vector<uint8_t> toDer() const;
  [[nodiscard]] std::string toPem() const;

  [[nodiscard]] Curve curve() const noexcept { return m_curve; }
  [[nodiscard]] std::span<const uint8_t> der() const noexcept {
    return {m_der.data(), m_der.size()};
  }
  [[nodiscard]] bool empty() const noexcept { return m_der.empty(); }

 private:
  friend class KeyPair;
  explicit PrivateKey(Curve curve, std::vector<uint8_t> der)
      : m_curve(curve), m_der(std::move(der)) {}

  void clear() noexcept;

  Curve m_curve{Curve::P256};
  std::vector<uint8_t> m_der;
};

class KeyPair final {
 public:
  KeyPair() = default;
  KeyPair(KeyPair&&) noexcept = default;
  KeyPair& operator=(KeyPair&&) noexcept = default;

  static KeyPair generate(Curve curve);

  [[nodiscard]] const PublicKey& publicKey() const noexcept { return m_public; }
  [[nodiscard]] const PrivateKey& privateKey() const noexcept {
    return m_private;
  }
  [[nodiscard]] Curve curve() const noexcept { return m_private.curve(); }

 private:
  KeyPair(PublicKey pub, PrivateKey priv)
      : m_public(std::move(pub)), m_private(std::move(priv)) {}

  PublicKey m_public;
  PrivateKey m_private;
};

[[nodiscard]] bool supportsSignature(
    Curve curve, const SignatureAlgorithm& algorithm) noexcept;
[[nodiscard]] bool supportsKeyAgreement(
    Curve curve, KeyAgreementAlgorithm algorithm) noexcept;

[[nodiscard]] SignatureAlgorithm preferredSignature(Curve curve);
[[nodiscard]] KeyAgreementAlgorithm preferredKeyAgreement(Curve curve);

Signature sign(const PrivateKey& key, std::span<const uint8_t> message,
               const SignatureAlgorithm& algorithm);

bool verify(const PublicKey& key, std::span<const uint8_t> message,
            const Signature& signature, const SignatureAlgorithm& algorithm);

Secret deriveSharedSecret(const PrivateKey& privateKey,
                          const PublicKey& publicKey,
                          KeyAgreementAlgorithm algorithm);

}  // namespace ikea400::crypto::curves