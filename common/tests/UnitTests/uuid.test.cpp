#include <gtest/gtest.h>
#include <utils/uuid.h>

#include <array>
#include <cstdint>
#include <exception>
#include <glaze/core/context.hpp>
#include <glaze/core/opts.hpp>
#include <glaze/core/read.hpp>
#include <glaze/core/write.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace ikea400::test {

// Helper to create a valid V4 raw array payload manually
static std::array<uint8_t, 16> createValidV4Bytes() {
  std::array<uint8_t, 16> data{};
  data.fill(0xAA);
  data[6] = (data[6] & 0x0F) | 0x40;  // Force Version 4
  data[8] = (data[8] & 0x3F) | 0x80;  // Force RFC 4122 Variant
  return data;
}

// -----------------------------------------------------------------------------
// 1. Constructors & Initialization
// -----------------------------------------------------------------------------

TEST(UuidTest, DefaultConstructorInitializesToNull) {
  uuid u;
  EXPECT_TRUE(u.isNull());
  EXPECT_EQ(u.bytes(), uuid::NULL_UUID);
}

TEST(UuidTest, NullStaticMethod) {
  uuid u = uuid::null();
  EXPECT_TRUE(u.isNull());
  EXPECT_EQ(u.bytes(), uuid::NULL_UUID);
}

TEST(UuidTest, ConstructorWithValidV4Bytes) {
  auto valid_bytes = createValidV4Bytes();
  uuid u(valid_bytes);

  EXPECT_FALSE(u.isNull());
  EXPECT_EQ(u.bytes(), valid_bytes);
}

TEST(UuidTest, ConstructorWithInvalidV4BytesResetsToNull) {
  std::array<uint8_t, 16> invalid_bytes{};
  invalid_bytes.fill(0xFF);  // Version will be 0xF instead of 0x4

  uuid u(invalid_bytes);
  EXPECT_TRUE(u.isNull());
  EXPECT_EQ(u.bytes(), uuid::NULL_UUID);
}

TEST(UuidTest, MoveConstructorWithValidBytes) {
  auto valid_bytes = createValidV4Bytes();
  auto bytes_copy = valid_bytes;

  uuid u(std::move(bytes_copy));
  EXPECT_FALSE(u.isNull());
  EXPECT_EQ(u.bytes(), valid_bytes);
}

// -----------------------------------------------------------------------------
// 2. Random Generation (V4 Metadata Verification)
// -----------------------------------------------------------------------------

TEST(UuidTest, RandomGenerationCreatesValidV4) {
  uuid u = uuid::rand();

  EXPECT_FALSE(u.isNull());

  // Validate version bits (byte 6 must match 0x4X)
  EXPECT_EQ(u.bytes()[6] & 0xF0, 0x40);

  // Validate variant bits (byte 8 must match 10XXXXXX -> top 2 bits must be
  // 0x80)
  EXPECT_EQ(u.bytes()[8] & 0xC0, 0x80);
}

TEST(UuidTest, RandomGenerationProducesUniqueValues) {
  std::unordered_set<std::string> generated_uuids;
  const size_t iterations = 100;

  for (size_t i = 0; i < iterations; ++i) {
    uuid u = uuid::rand();
    generated_uuids.insert(u.toString());
  }

  // If every random uuid is unique, the set size matches the iteration count
  EXPECT_EQ(generated_uuids.size(), iterations);
}

// -----------------------------------------------------------------------------
// 3. String Conversions (toString & fromString)
// -----------------------------------------------------------------------------

TEST(UuidTest, ToStringOnNullUuid) {
  uuid u;
  EXPECT_EQ(u.toString(), "00000000-0000-0000-0000-000000000000");
  EXPECT_EQ(u.toString<true>(), "00000000-0000-0000-0000-000000000000");
}

TEST(UuidTest, ToStringFormatsAndCasing) {
  // Explicitly seed bytes to predictable hex tokens
  std::array<uint8_t, 16> bytes = {0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x6f,
                                   0x4a, 0x7b, 0x8c, 0x9d, 0x0a, 0x1b,
                                   0x2c, 0x3d, 0x4e, 0x5f};
  // Ensure version (4) and variant (8) bits are properly forced for validity
  bytes[6] = (bytes[6] & 0x0F) | 0x40;
  bytes[8] = (bytes[8] & 0x3F) | 0x80;

  uuid u(bytes);

  EXPECT_EQ(u.toString<false>(), "1a2b3c4d-5e6f-4a7b-8c9d-0a1b2c3d4e5f");
  EXPECT_EQ(u.toString<true>(), "1A2B3C4D-5E6F-4A7B-8C9D-0A1B2C3D4E5F");
}

TEST(UuidTest, FromStringWithValidString) {
  std::string_view valid_str = "f81d4fae-7dec-11d0-a765-00a0c91e6bf6";
  // Tweak to satisfy valid V4 restrictions manually
  std::string valid_v4_str = "f81d4fae-7dec-41d0-a765-00a0c91e6bf6";

  uuid u = uuid::fromString<true>(valid_v4_str);
  EXPECT_FALSE(u.isNull());
  EXPECT_EQ(u.toString(), valid_v4_str);
}

TEST(UuidTest, FromStringAcceptsUppercase) {
  std::string uppercase_v4_str = "F81D4FAE-7DEC-41D0-A765-00A0C91E6BF6";

  uuid u = uuid::fromString<true>(uppercase_v4_str);
  EXPECT_FALSE(u.isNull());
  EXPECT_EQ(u.toString<true>(), uppercase_v4_str);
}

TEST(UuidTest, FromStringThrowingOnInvalidFormats) {
  // Invalid size
  EXPECT_THROW(uuid::fromString<true>("f81d4fae-7dec-41d0-a765-00a0c91e6b"),
               std::exception);
  // Missing/misplaced dashes
  EXPECT_THROW(uuid::fromString<true>("f81d4fae7dec41d0a76500a0c91e6bf60000"),
               std::exception);
  // Non-hex characters
  EXPECT_THROW(uuid::fromString<true>("f81d4fae-7dec-41d0-a765-00a0c91e6bG6"),
               std::exception);
  // Invalid V4 metadata (wrong version)
  EXPECT_THROW(uuid::fromString<true>("f81d4fae-7dec-31d0-a765-00a0c91e6bf6"),
               std::exception);
}

TEST(UuidTest, FromStringNonThrowingOnInvalidFormats) {
  // Should gracefully return null instead of crashing
  uuid u1 = uuid::fromString<false>("invalid-format-string");
  EXPECT_TRUE(u1.isNull());

  uuid u2 = uuid::fromString<false>(
      "f81d4fae-7dec-31d0-a765-00a0c91e6bf6");  // Invalid version
  EXPECT_TRUE(u2.isNull());
}

// -----------------------------------------------------------------------------
// 4. Comparison Operators & STL Hashing
// -----------------------------------------------------------------------------

TEST(UuidTest, EqualityAndInequalityOperators) {
  uuid u1 = uuid::rand();
  uuid u2 = u1;
  uuid u3 = uuid::rand();

  EXPECT_TRUE(u1 == u2);
  EXPECT_FALSE(u1 == u3);
}

TEST(UuidTest, StdHashCompatibility) {
  uuid u1 = uuid::rand();
  uuid u2 = u1;

  std::hash<uuid> uuid_hasher;
  EXPECT_EQ(uuid_hasher(u1), uuid_hasher(u2));

  // Confirm we can insert it cleanly into an unordered_set
  std::unordered_set<uuid> uuid_set;
  uuid_set.insert(u1);
  EXPECT_TRUE(uuid_set.contains(u2));
}

// -----------------------------------------------------------------------------
// 5. Glaze JSON Serialization / Deserialization
// -----------------------------------------------------------------------------

TEST(UuidTest, GlazeJsonSerializationAndDeserialization) {
  uuid original = uuid::rand();

  // Serialize to JSON string buffer via Glaze
  std::string json_buffer;
  auto ec_serialize = glz::write<glz::opts{}>(original, json_buffer);
  ASSERT_FALSE(ec_serialize);  // Glaze returns an error context object which
                               // evaluates to false on success

  // Verify it added surrounding quotes for a JSON string
  EXPECT_EQ(json_buffer, "\"" + original.toString() + "\"");

  // Deserialize back into a fresh instance
  uuid deserialized;
  auto ec_deserialize = glz::read<glz::opts{}>(deserialized, json_buffer);
  ASSERT_FALSE(ec_deserialize);

  EXPECT_EQ(original, deserialized);
}

TEST(UuidTest, GlazeJsonDeserializationFailsOnInvalidUuid) {
  std::string invalid_json_uuid = "\"not-a-valid-uuid-format-at-all\"";
  uuid target;

  auto ec = glz::read<glz::opts{}>(target, invalid_json_uuid);

  // The Glaze implementation sets glz::error_code::parse_error on failure
  EXPECT_TRUE(ec);
  EXPECT_EQ(ec.ec, glz::error_code::parse_error);
}

}  // namespace ikea400::test