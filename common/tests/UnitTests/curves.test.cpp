#include <crypto/curves.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <ranges>
#include <vector>

namespace {
using namespace ikea400::crypto::curves;
using ikea400::crypto::HashAlgo;

struct SignatureCase {
  Curve curve;
  SignatureAlgorithm algorithm;
};

class SignatureTests : public testing::TestWithParam<SignatureCase> {};

INSTANTIATE_TEST_SUITE_P(
    CurveSignatures, SignatureTests,
    testing::Values(
        SignatureCase{Curve::P256, SignatureAlgorithm::ecdsa(HashAlgo::Sha256)},
        SignatureCase{Curve::P384, SignatureAlgorithm::ecdsa(HashAlgo::Sha384)},
        SignatureCase{Curve::P521, SignatureAlgorithm::ecdsa(HashAlgo::Sha512)},
        SignatureCase{Curve::Secp256k1,
                      SignatureAlgorithm::ecdsa(HashAlgo::Sha256)},
        SignatureCase{Curve::Ed25519, SignatureAlgorithm::eddsa()}));

TEST_P(SignatureTests, SignAndVerify) {
  const auto [curve, algorithm] = GetParam();

  auto keypair = KeyPair::generate(curve);

  std::array<uint8_t, 32> message{};
  for (size_t i = 0; i < message.size(); ++i) {
    message[i] = static_cast<uint8_t>(i + 1);
  }

  auto signature = sign(keypair.privateKey(), message, algorithm);
  EXPECT_TRUE(verify(keypair.publicKey(), message, signature, algorithm));

  message[0] ^= 0xFF;
  EXPECT_FALSE(verify(keypair.publicKey(), message, signature, algorithm));
}

struct AgreementCase {
  Curve curve;
  KeyAgreementAlgorithm algorithm;
};

class AgreementTests : public testing::TestWithParam<AgreementCase> {};

INSTANTIATE_TEST_SUITE_P(
    CurveAgreement, AgreementTests,
    testing::Values(AgreementCase{Curve::P256, KeyAgreementAlgorithm::Ecdh},
                    AgreementCase{Curve::X25519, KeyAgreementAlgorithm::Xdh}));

TEST_P(AgreementTests, SharedSecretMatches) {
  const auto [curve, algorithm] = GetParam();

  auto alice = KeyPair::generate(curve);
  auto bob = KeyPair::generate(curve);

  auto secretA =
      deriveSharedSecret(alice.privateKey(), bob.publicKey(), algorithm);
  auto secretB =
      deriveSharedSecret(bob.privateKey(), alice.publicKey(), algorithm);

  EXPECT_EQ(secretA.size(), secretB.size());
  EXPECT_TRUE(std::ranges::equal(secretA.bytes(), secretB.bytes()));
}

TEST(CurveKeyRoundTrip, ExportImport) {
  auto keypair = KeyPair::generate(Curve::P256);

  auto pubDer = keypair.publicKey().toDer();
  auto privDer = keypair.privateKey().toDer();

  auto pub = PublicKey::fromDer(pubDer);
  auto priv = PrivateKey::fromDer(privDer);

  std::array<uint8_t, 16> message{1, 2,  3,  4,  5,  6,  7,  8,
                                  9, 10, 11, 12, 13, 14, 15, 16};
  auto algorithm = SignatureAlgorithm::ecdsa(HashAlgo::Sha256);

  auto signature = sign(priv, message, algorithm);
  EXPECT_TRUE(verify(pub, message, signature, algorithm));
}

}  // namespace