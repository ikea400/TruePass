#include <gtest/gtest.h>
#include <utils/BinEncoding.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace ikea400::bin::test {

// -----------------------------------------------------------------------------
// 1. Hex Encoding Tests
// -----------------------------------------------------------------------------

TEST(HexEncodingTest, EmptyInputReturnsEmptyString) {
  std::vector<uint8_t> empty_vec;
  EXPECT_TRUE(hex::encode(empty_vec).empty());
  EXPECT_EQ(hex::format(empty_vec), "hex:");
}

TEST(HexEncodingTest, StandardVectorEncoding) {
  std::vector<uint8_t> input = {0x00, 0x01, 0x0A, 0x0F, 0xFF, 0x80};
  std::string expected =
      "00010a0fff80";  // Note: Assumes lower-case from underlying fast-hex

  // Test raw encode
  EXPECT_EQ(hex::encode(input), expected);

  // Test formatted encode
  EXPECT_EQ(hex::format(input), "hex:" + expected);
}

TEST(HexEncodingTest, StandardStringEncoding) {
  std::string input = "Hello";  // 0x48, 0x65, 0x6c, 0x6c, 0x6f
  std::string expected = "48656c6c6f";

  EXPECT_EQ(hex::encode(input), expected);
  EXPECT_EQ(hex::format(input), "hex:" + expected);
}

// -----------------------------------------------------------------------------
// 2. Hex Decoding Tests
// -----------------------------------------------------------------------------

TEST(HexDecodingTest, EmptyInputReturnsEmptyVector) {
  auto res = hex::decode("");
  EXPECT_TRUE(res.empty());
}

TEST(HexDecodingTest, DecodeValidStrings) {
  std::string_view hex_str = "48656c6c6f00ff";
  std::vector<uint8_t> expected = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x00, 0xFF};

  EXPECT_EQ(hex::decode(hex_str), expected);
}

TEST(HexDecodingTest, DecodeOddSizedStringThrowsOrReturnsEmpty) {
  std::string_view odd_hex = "A";  // Odd length is invalid

  // Throw branch (default)
  EXPECT_THROW(hex::decode<true>(odd_hex), std::exception);

  // Non-throw branch
  auto res = hex::decode<false>(odd_hex);
  EXPECT_TRUE(res.empty());
}

TEST(HexDecodingTest, DecodeFormatSuccess) {
  std::string_view formatted = "hex:48656c6c6f";
  std::vector<uint8_t> expected = {0x48, 0x65, 0x6c, 0x6c, 0x6f};

  EXPECT_EQ(hex::decodeFormat<true>(formatted), expected);
}

TEST(HexDecodingTest, DecodeFormatInvalidPrefix) {
  std::string_view wrong_prefix = "raw:48656c6c6f";

  EXPECT_THROW(hex::decodeFormat<true>(wrong_prefix), std::exception);

  auto res = hex::decodeFormat<false>(wrong_prefix);
  EXPECT_TRUE(res.empty());
}

// -----------------------------------------------------------------------------
// 3. Base64 Encoding Tests
// -----------------------------------------------------------------------------

TEST(Base64EncodingTest, EmptyInputReturnsEmpty) {
  std::vector<uint8_t> empty;
  EXPECT_TRUE(b64::encode(empty).empty());
  EXPECT_EQ(b64::format(empty), "b64:");
}

TEST(Base64EncodingTest, StandardVectorEncoding) {
  // "Many hands make light work." snippet standard token
  std::vector<uint8_t> input = {'M', 'a', 'n', 'y'};
  std::string expected = "TWFueQ==";

  EXPECT_EQ(b64::encode(input), expected);
  EXPECT_EQ(b64::format(input), "b64:" + expected);
}

// -----------------------------------------------------------------------------
// 4. Base64 Decoding Tests
// -----------------------------------------------------------------------------

TEST(Base64DecodingTest, DecodeValidStrings) {
  std::string_view encoded = "TWFueQ==";
  std::vector<uint8_t> expected = {'M', 'a', 'n', 'y'};

  EXPECT_EQ(b64::decode<true>(encoded), expected);
  EXPECT_EQ(b64::decode<false>(encoded), expected);
}

TEST(Base64DecodingTest, DecodeInvalidStringsConditionalThrow) {
  std::string_view bad_b64 = "!!!NotValidBase64!!!";

// If your underlying base64 library throws on bad padding/chars:
// This will catch structural library throws converted to safety fallback
// options
try {
  auto res = b64::decode<false>(bad_b64);
  // If it didn't throw, it should either pass or return an empty vector
  // fallback
  SUCCEED();
}
catch (...) {
  FAIL() << "b64::decode<false> leaked an exception!";
}
}  // namespace ikea400::bin::test

TEST(Base64DecodingTest, DecodeFormatSuccessAndFailure) {
  std::string_view valid_formatted = "b64:TWFueQ==";
  std::vector<uint8_t> expected = {'M', 'a', 'n', 'y'};

  EXPECT_EQ(b64::decodeFormat(valid_formatted), expected);
  EXPECT_THROW(b64::decodeFormat("hex:TWFueQ=="), std::exception);
}

// -----------------------------------------------------------------------------
// 5. Global Unified DecodeFormat Template Tests
// -----------------------------------------------------------------------------

TEST(GlobalDecodeFormatTest, DispatchesToHexCorrectly) {
  std::string_view hex_input = "hex:48656c6c6f";
  std::vector<uint8_t> expected = {0x48, 0x65, 0x6c, 0x6c, 0x6f};

  EXPECT_EQ(decodeFormat<true>(hex_input), expected);
  EXPECT_EQ(decodeFormat<false>(hex_input), expected);
}

TEST(GlobalDecodeFormatTest, DispatchesToBase64Correctly) {
  std::string_view b64_input = "b64:TWFueQ==";
  std::vector<uint8_t> expected = {'M', 'a', 'n', 'y'};

  EXPECT_EQ(decodeFormat<true>(b64_input), expected);
  EXPECT_EQ(decodeFormat<false>(b64_input), expected);
}

TEST(GlobalDecodeFormatTest, InvalidPrefixHandling) {
  std::string_view bad_prefix = "unknown:data";

  EXPECT_THROW(decodeFormat<true>(bad_prefix), std::exception);

  auto res = decodeFormat<false>(bad_prefix);
  EXPECT_TRUE(res.empty());
}

}  // namespace ikea400::bin::test