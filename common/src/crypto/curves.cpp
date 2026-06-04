#include "../../include/crypto/curves.h"

#include <crypto/hash_algo.h>
#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/core_names.h>
#include <openssl/pem.h>
#include <openssl/types.h>
#include <openssl/x509.h>

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../../include/utils/utils.h"

namespace ikea400::crypto::curves {
namespace {

using EVP_PKEY_ptr =
    std::unique_ptr<EVP_PKEY, utils::DeleterFromFn<&::EVP_PKEY_free>>;
using EVP_PKEY_CTX_ptr =
    std::unique_ptr<EVP_PKEY_CTX, utils::DeleterFromFn<&::EVP_PKEY_CTX_free>>;
using EVP_MD_CTX_ptr =
    std::unique_ptr<EVP_MD_CTX, utils::DeleterFromFn<&::EVP_MD_CTX_free>>;
using BIO_ptr = std::unique_ptr<BIO, utils::DeleterFromFn<&::BIO_free>>;
using PKCS8_ptr =
    std::unique_ptr<PKCS8_PRIV_KEY_INFO,
                    utils::DeleterFromFn<&::PKCS8_PRIV_KEY_INFO_free>>;

std::string buildError(const char* context) {
  unsigned long err = ERR_get_error();
  if (!err) return context;
  std::array<char, 256> buffer{};
  ERR_error_string_n(err, buffer.data(), buffer.size());
  return std::string(context) + ": " + buffer.data();
}

[[noreturn]] void throwError(const char* context) {
  throw CryptoError(buildError(context));
}

const EVP_MD* mapHash(HashAlgo algo) {
  switch (algo) {
    case HashAlgo::Sha1:
      return EVP_sha1();
    case HashAlgo::Sha256:
      return EVP_sha256();
    case HashAlgo::Sha384:
      return EVP_sha384();
    case HashAlgo::Sha512:
      return EVP_sha512();
    case HashAlgo::Sha3_256:
      return EVP_sha3_256();
    case HashAlgo::Sha3_384:
      return EVP_sha3_384();
    case HashAlgo::Sha3_512:
      return EVP_sha3_512();
    case HashAlgo::Blake2b512:
      return EVP_blake2b512();
    case HashAlgo::Blake2s256:
      return EVP_blake2s256();
    default:
      break;
  }
  return nullptr;
}

int curveToNid(Curve curve) {
  switch (curve) {
    case Curve::P256:
      return NID_X9_62_prime256v1;
    case Curve::P384:
      return NID_secp384r1;
    case Curve::P521:
      return NID_secp521r1;
    case Curve::Secp256k1:
      return NID_secp256k1;
    default:
      break;
  }
  return NID_undef;
}

Curve curveFromNid(int nid) {
  switch (nid) {
    case NID_X9_62_prime256v1:
      return Curve::P256;
    case NID_secp384r1:
      return Curve::P384;
    case NID_secp521r1:
      return Curve::P521;
    case NID_secp256k1:
      return Curve::Secp256k1;
    default:
      break;
  }
  throw CryptoError("Courbe EC non supportée.");
}

Curve curveFromPkey(EVP_PKEY* key) {
  const int id = EVP_PKEY_id(key);
  switch (id) {
    case EVP_PKEY_EC: {
      char groupName[80] = {};

      const size_t len =
          EVP_PKEY_get_utf8_string_param(key, OSSL_PKEY_PARAM_GROUP_NAME,
                                         groupName, sizeof(groupName), nullptr);

      if (len == 0) {
        throw CryptoError("Impossible d'obtenir le groupe EC.");
      }

      const int nid = OBJ_sn2nid(groupName);
      if (nid == NID_undef) {
        throw CryptoError("Courbe EC inconnue.");
      }

      return curveFromNid(nid);
    }
    case EVP_PKEY_ED25519:
      return Curve::Ed25519;
    case EVP_PKEY_ED448:
      return Curve::Ed448;
    case EVP_PKEY_X25519:
      return Curve::X25519;
    case EVP_PKEY_X448:
      return Curve::X448;
    default:
      break;
  }

  throw CryptoError("Type de clé non supporté.");
}

EVP_PKEY_ptr generateKey(Curve curve) {
  auto generateFromName = [](const char* name) {
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new_from_name(nullptr, name, nullptr));
    if (!ctx) throwError("Création du contexte");
    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) throwError("Init keygen");
    EVP_PKEY* out = nullptr;
    if (EVP_PKEY_keygen(ctx.get(), &out) <= 0) throwError("Keygen");
    return EVP_PKEY_ptr(out);
  };

  auto generateFromNid = [](int nid) {
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr));
    if (!ctx) throwError("Création du contexte EC");
    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) throwError("Init EC");
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx.get(), nid) <= 0)
      throwError("Paramètres EC");
    if (EVP_PKEY_CTX_set_ec_param_enc(ctx.get(), OPENSSL_EC_NAMED_CURVE) <= 0)
      throwError("Encodage EC");
    EVP_PKEY* out = nullptr;
    if (EVP_PKEY_keygen(ctx.get(), &out) <= 0) throwError("Keygen EC");
    return EVP_PKEY_ptr(out);
  };

  switch (curve) {
    case Curve::Ed25519:
      return generateFromName("ED25519");
    case Curve::Ed448:
      return generateFromName("ED448");
    case Curve::X25519:
      return generateFromName("X25519");
    case Curve::X448:
      return generateFromName("X448");
    case Curve::P256:
    case Curve::P384:
    case Curve::P521:
    case Curve::Secp256k1:
      return generateFromNid(curveToNid(curve));
  }

  throwError("Curve not supported");
}

std::vector<uint8_t> exportPublicDer(EVP_PKEY* key) {
  int size = i2d_PUBKEY(key, nullptr);
  if (size <= 0) throwError("Export DER public");
  std::vector<uint8_t> der(static_cast<size_t>(size));
  unsigned char* ptr = der.data();
  if (i2d_PUBKEY(key, &ptr) != size) throwError("Export DER public");
  return der;
}

std::vector<uint8_t> exportPrivateDer(EVP_PKEY* key) {
  PKCS8_ptr p8(EVP_PKEY2PKCS8(key));
  if (!p8) throwError("Conversion PKCS#8");
  int size = i2d_PKCS8_PRIV_KEY_INFO(p8.get(), nullptr);
  if (size <= 0) throwError("Export DER privé");
  std::vector<uint8_t> der(static_cast<size_t>(size));
  unsigned char* ptr = der.data();
  if (i2d_PKCS8_PRIV_KEY_INFO(p8.get(), &ptr) != size)
    throwError("Export DER privé");
  return der;
}

EVP_PKEY_ptr importPublicDer(std::span<const uint8_t> der) {
  const unsigned char* ptr = der.data();
  EVP_PKEY* key = d2i_PUBKEY(nullptr, &ptr, static_cast<long>(der.size()));
  if (!key) throwError("Import DER public");
  return EVP_PKEY_ptr(key);
}

EVP_PKEY_ptr importPrivateDer(std::span<const uint8_t> der) {
  const unsigned char* ptr = der.data();
  PKCS8_PRIV_KEY_INFO* p8 =
      d2i_PKCS8_PRIV_KEY_INFO(nullptr, &ptr, static_cast<long>(der.size()));
  if (!p8) throwError("Import PKCS#8");
  PKCS8_ptr p8ptr(p8);
  EVP_PKEY* key = EVP_PKCS82PKEY(p8ptr.get());
  if (!key) throwError("Import clé privée");
  return EVP_PKEY_ptr(key);
}

std::string exportPublicPem(EVP_PKEY* key) {
  BIO_ptr mem(BIO_new(BIO_s_mem()));
  if (!mem) throwError("BIO public");
  if (PEM_write_bio_PUBKEY(mem.get(), key) <= 0)
    throwError("Export PEM public");
  BUF_MEM* buffer = nullptr;
  BIO_get_mem_ptr(mem.get(), &buffer);
  return std::string(buffer->data, buffer->length);
}

std::string exportPrivatePem(EVP_PKEY* key) {
  BIO_ptr mem(BIO_new(BIO_s_mem()));
  if (!mem) throwError("BIO privé");
  if (PEM_write_bio_PKCS8PrivateKey(mem.get(), key, nullptr, nullptr, 0,
                                    nullptr, nullptr) <= 0)
    throwError("Export PEM privé");
  BUF_MEM* buffer = nullptr;
  BIO_get_mem_ptr(mem.get(), &buffer);
  return std::string(buffer->data, buffer->length);
}

EVP_PKEY_ptr importPublicPem(std::string_view pem) {
  BIO_ptr mem(BIO_new_mem_buf(pem.data(), static_cast<int>(pem.size())));
  if (!mem) throwError("BIO import PEM public");
  EVP_PKEY* key = PEM_read_bio_PUBKEY(mem.get(), nullptr, nullptr, nullptr);
  if (!key) throwError("Import PEM public");
  return EVP_PKEY_ptr(key);
}

EVP_PKEY_ptr importPrivatePem(std::string_view pem) {
  BIO_ptr mem(BIO_new_mem_buf(pem.data(), static_cast<int>(pem.size())));
  if (!mem) throwError("BIO import PEM privé");
  EVP_PKEY* key = PEM_read_bio_PrivateKey(mem.get(), nullptr, nullptr, nullptr);
  if (!key) throwError("Import PEM privé");
  return EVP_PKEY_ptr(key);
}

EVP_PKEY_ptr keyFromPrivate(const PrivateKey& key) {
  if (key.empty()) throw CryptoError("Clé privée vide.");
  auto pkey = importPrivateDer(key.der());
  if (curveFromPkey(pkey.get()) != key.curve())
    throw CryptoError("Courbe incohérente (clé privée).");
  return pkey;
}

EVP_PKEY_ptr keyFromPublic(const PublicKey& key) {
  if (key.empty()) throw CryptoError("Clé publique vide.");
  auto pkey = importPublicDer(key.der());
  if (curveFromPkey(pkey.get()) != key.curve())
    throw CryptoError("Courbe incohérente (clé publique).");
  return pkey;
}

}  // namespace

std::string_view toString(Curve curve) noexcept {
  switch (curve) {
    case Curve::P256:
      return "P-256";
    case Curve::P384:
      return "P-384";
    case Curve::P521:
      return "P-521";
    case Curve::Secp256k1:
      return "secp256k1";
    case Curve::Ed25519:
      return "Ed25519";
    case Curve::Ed448:
      return "Ed448";
    case Curve::X25519:
      return "X25519";
    case Curve::X448:
      return "X448";
  }
  return "Unknown";
}

SignatureAlgorithm SignatureAlgorithm::ecdsa(HashAlgo hash) {
  if (hash == HashAlgo::None) throw CryptoError("Hash ECDSA invalide.");
  return SignatureAlgorithm(SignatureAlgorithmKind::Ecdsa, hash);
}

SignatureAlgorithm SignatureAlgorithm::eddsa() {
  return SignatureAlgorithm(SignatureAlgorithmKind::EdDsa, HashAlgo::None);
}

Secret::Secret(Secret&& other) noexcept : m_data(std::move(other.m_data)) {
  other.clear();
}

Secret& Secret::operator=(Secret&& other) noexcept {
  if (this != &other) {
    clear();
    m_data = std::move(other.m_data);
    other.clear();
  }
  return *this;
}

Secret::~Secret() noexcept { clear(); }

void Secret::clear() noexcept {
  if (!m_data.empty()) {
    utils::secureErase(m_data);
    m_data.clear();
  }
}

PrivateKey::PrivateKey(PrivateKey&& other) noexcept
    : m_curve(other.m_curve), m_der(std::move(other.m_der)) {
  other.clear();
}

PrivateKey& PrivateKey::operator=(PrivateKey&& other) noexcept {
  if (this != &other) {
    clear();
    m_curve = other.m_curve;
    m_der = std::move(other.m_der);
    other.clear();
  }
  return *this;
}

PrivateKey::~PrivateKey() noexcept { clear(); }

void PrivateKey::clear() noexcept {
  if (!m_der.empty()) {
    utils::secureErase(m_der);
    m_der.clear();
  }
}

PublicKey PublicKey::fromDer(std::span<const uint8_t> der) {
  if (der.empty()) throw CryptoError("DER public vide.");
  auto pkey = importPublicDer(der);
  return PublicKey(curveFromPkey(pkey.get()),
                   std::vector<uint8_t>(der.begin(), der.end()));
}

PublicKey PublicKey::fromPem(std::string_view pem) {
  if (pem.empty()) throw CryptoError("PEM public vide.");
  auto pkey = importPublicPem(pem);
  return PublicKey(curveFromPkey(pkey.get()), exportPublicDer(pkey.get()));
}

std::vector<uint8_t> PublicKey::toDer() const {
  if (m_der.empty()) throw CryptoError("Clé publique vide.");
  return m_der;
}

std::string PublicKey::toPem() const {
  auto pkey = keyFromPublic(*this);
  return exportPublicPem(pkey.get());
}

PrivateKey PrivateKey::fromDer(std::span<const uint8_t> der) {
  if (der.empty()) throw CryptoError("DER privé vide.");
  auto pkey = importPrivateDer(der);
  return PrivateKey(curveFromPkey(pkey.get()),
                    std::vector<uint8_t>(der.begin(), der.end()));
}

PrivateKey PrivateKey::fromPem(std::string_view pem) {
  if (pem.empty()) throw CryptoError("PEM privé vide.");
  auto pkey = importPrivatePem(pem);
  return PrivateKey(curveFromPkey(pkey.get()), exportPrivateDer(pkey.get()));
}

std::vector<uint8_t> PrivateKey::toDer() const {
  if (m_der.empty()) throw CryptoError("Clé privée vide.");
  return m_der;
}

std::string PrivateKey::toPem() const {
  auto pkey = keyFromPrivate(*this);
  return exportPrivatePem(pkey.get());
}

KeyPair KeyPair::generate(Curve curve) {
  auto pkey = generateKey(curve);
  return KeyPair(PublicKey(curve, exportPublicDer(pkey.get())),
                 PrivateKey(curve, exportPrivateDer(pkey.get())));
}

bool supportsSignature(Curve curve,
                       const SignatureAlgorithm& algorithm) noexcept {
  if (algorithm.kind() == SignatureAlgorithmKind::Ecdsa) {
    return curve == Curve::P256 || curve == Curve::P384 ||
           curve == Curve::P521 || curve == Curve::Secp256k1;
  }
  return curve == Curve::Ed25519 || curve == Curve::Ed448;
}

bool supportsKeyAgreement(Curve curve,
                          KeyAgreementAlgorithm algorithm) noexcept {
  if (algorithm == KeyAgreementAlgorithm::Ecdh) {
    return curve == Curve::P256 || curve == Curve::P384 ||
           curve == Curve::P521 || curve == Curve::Secp256k1;
  }
  return curve == Curve::X25519 || curve == Curve::X448;
}

SignatureAlgorithm preferredSignature(Curve curve) {
  if (curve == Curve::Ed25519 || curve == Curve::Ed448) {
    return SignatureAlgorithm::eddsa();
  }
  if (curve == Curve::P384) return SignatureAlgorithm::ecdsa(HashAlgo::Sha384);
  if (curve == Curve::P521) return SignatureAlgorithm::ecdsa(HashAlgo::Sha512);
  return SignatureAlgorithm::ecdsa(HashAlgo::Sha256);
}

KeyAgreementAlgorithm preferredKeyAgreement(Curve curve) {
  if (curve == Curve::X25519 || curve == Curve::X448)
    return KeyAgreementAlgorithm::Xdh;
  return KeyAgreementAlgorithm::Ecdh;
}

Signature sign(const PrivateKey& key, std::span<const uint8_t> message,
               const SignatureAlgorithm& algorithm) {
  if (!supportsSignature(key.curve(), algorithm))
    throw CryptoError("Algorithme de signature incompatible.");

  auto pkey = keyFromPrivate(key);
  EVP_MD_CTX_ptr ctx(EVP_MD_CTX_new());
  if (!ctx) throwError("Création contexte signature");

  const EVP_MD* md = nullptr;
  if (algorithm.kind() == SignatureAlgorithmKind::Ecdsa) {
    md = mapHash(algorithm.hash());
    if (!md) throw CryptoError("Hash ECDSA non supporté.");
  }

  if (EVP_DigestSignInit(ctx.get(), nullptr, md, nullptr, pkey.get()) <= 0)
    throwError("Init signature");

  size_t sigLen = 0;
  if (EVP_DigestSign(ctx.get(), nullptr, &sigLen, message.data(),
                     message.size()) <= 0)
    throwError("Taille signature");

  std::vector<uint8_t> sig(sigLen);
  if (EVP_DigestSign(ctx.get(), sig.data(), &sigLen, message.data(),
                     message.size()) <= 0)
    throwError("Signature");
  sig.resize(sigLen);

  return Signature(std::move(sig));
}

bool verify(const PublicKey& key, std::span<const uint8_t> message,
            const Signature& signature, const SignatureAlgorithm& algorithm) {
  if (!supportsSignature(key.curve(), algorithm)) return false;
  if (signature.empty()) return false;

  auto pkey = keyFromPublic(key);
  EVP_MD_CTX_ptr ctx(EVP_MD_CTX_new());
  if (!ctx) throwError("Création contexte vérif");

  const EVP_MD* md = nullptr;
  if (algorithm.kind() == SignatureAlgorithmKind::Ecdsa) {
    md = mapHash(algorithm.hash());
    if (!md) throw CryptoError("Hash ECDSA non supporté.");
  }

  if (EVP_DigestVerifyInit(ctx.get(), nullptr, md, nullptr, pkey.get()) <= 0)
    throwError("Init vérif");

  const auto sigSpan = signature.bytes();
  const int result = EVP_DigestVerify(ctx.get(), sigSpan.data(), sigSpan.size(),
                                      message.data(), message.size());
  return result == 1;
}

Secret deriveSharedSecret(const PrivateKey& privateKey,
                          const PublicKey& publicKey,
                          KeyAgreementAlgorithm algorithm) {
  if (!supportsKeyAgreement(privateKey.curve(), algorithm) ||
      privateKey.curve() != publicKey.curve()) {
    throw CryptoError("Échange de clés incompatible.");
  }

  auto priv = keyFromPrivate(privateKey);
  auto pub = keyFromPublic(publicKey);

  EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new(priv.get(), nullptr));
  if (!ctx) throwError("Contexte dérivation");

  if (EVP_PKEY_derive_init(ctx.get()) <= 0) throwError("Init dérivation");
  if (EVP_PKEY_derive_set_peer(ctx.get(), pub.get()) <= 0)
    throwError("Définition du pair");

  size_t secretLen = 0;
  if (EVP_PKEY_derive(ctx.get(), nullptr, &secretLen) <= 0)
    throwError("Taille secret");

  std::vector<uint8_t> secret(secretLen);
  if (EVP_PKEY_derive(ctx.get(), secret.data(), &secretLen) <= 0)
    throwError("Dérivation secret");
  secret.resize(secretLen);

  return Secret(std::move(secret));
}

}  // namespace ikea400::crypto::curves