// Host-side unit tests for seeed::mmwave::FrameCodec.
//
// Run with `pio test -e native`. These tests intentionally avoid Arduino.h
// so they can execute anywhere a C++17 compiler is available.

#include <unity.h>

#include <cstdint>
#include <cstring>
#include <vector>

#include "FrameCodec.h"

using seeed::mmwave::DecodedFrame;
using seeed::mmwave::FrameCodec;
using seeed::mmwave::Status;

static std::vector<uint8_t> makeFrame(uint16_t id, uint16_t type,
                                      const std::vector<uint8_t>& data) {
  std::vector<uint8_t> buf(FrameCodec::kHeaderSize + data.size() +
                           FrameCodec::kDataChecksum);
  size_t written = 0;
  Status st = FrameCodec::encode(id, type, data.data(), data.size(), buf.data(),
                                 buf.size(), written);
  TEST_ASSERT_EQUAL(static_cast<int>(Status::Ok), static_cast<int>(st));
  TEST_ASSERT_EQUAL_size_t(buf.size(), written);
  return buf;
}

void setUp(void) {}
void tearDown(void) {}

void test_checksum_is_xor_then_invert(void) {
  // ~(0x01 ^ 0x02 ^ 0x03) = ~0x00 = 0xFF
  const uint8_t a[] = {0x01, 0x02, 0x03};
  TEST_ASSERT_EQUAL_HEX8(0xFF, FrameCodec::checksum(a, sizeof(a)));

  // Empty input -> ~0x00 = 0xFF
  TEST_ASSERT_EQUAL_HEX8(0xFF, FrameCodec::checksum(nullptr, 0));
}

void test_encode_layout_is_stable(void) {
  const std::vector<uint8_t> payload = {0xDE, 0xAD, 0xBE, 0xEF};
  auto frame = makeFrame(/*id=*/0x1234, /*type=*/0x0105, payload);

  TEST_ASSERT_EQUAL_HEX8(FrameCodec::kSofByte, frame[0]);
  TEST_ASSERT_EQUAL_HEX8(0x12, frame[1]);  // id high
  TEST_ASSERT_EQUAL_HEX8(0x34, frame[2]);  // id low
  TEST_ASSERT_EQUAL_HEX8(0x00, frame[3]);  // len high
  TEST_ASSERT_EQUAL_HEX8(0x04, frame[4]);  // len low
  TEST_ASSERT_EQUAL_HEX8(0x01, frame[5]);  // type high
  TEST_ASSERT_EQUAL_HEX8(0x05, frame[6]);  // type low
  TEST_ASSERT_EQUAL_HEX8(FrameCodec::checksum(frame.data(), 7), frame[7]);
  TEST_ASSERT_EQUAL_HEX8(0xDE, frame[8]);
  TEST_ASSERT_EQUAL_HEX8(FrameCodec::checksum(frame.data() + 8, 4), frame[12]);
}

void test_encode_decode_roundtrip(void) {
  const std::vector<uint8_t> payload = {1, 2, 3, 4, 5, 6, 7, 8};
  auto frame = makeFrame(0xBEEF, 0x0200, payload);

  DecodedFrame out{};
  TEST_ASSERT_EQUAL(static_cast<int>(Status::Ok),
                    static_cast<int>(FrameCodec::tryDecode(
                        frame.data(), frame.size(), out)));
  TEST_ASSERT_EQUAL_UINT16(0xBEEF, out.id);
  TEST_ASSERT_EQUAL_UINT16(0x0200, out.type);
  TEST_ASSERT_EQUAL_UINT16(payload.size(), out.data_len);
  TEST_ASSERT_EQUAL_MEMORY(payload.data(), out.data, payload.size());
}

void test_decode_rejects_bad_sof(void) {
  auto frame = makeFrame(0x0001, 0x0001, {0xAA});
  frame[0] = 0x02;  // corrupt SOF
  DecodedFrame out{};
  TEST_ASSERT_EQUAL(static_cast<int>(Status::BadSof),
                    static_cast<int>(FrameCodec::tryDecode(
                        frame.data(), frame.size(), out)));
}

void test_decode_rejects_short_buffer(void) {
  auto frame = makeFrame(0x0001, 0x0001, {0xAA, 0xBB});
  DecodedFrame out{};
  // Trim off the final data_cksum.
  TEST_ASSERT_EQUAL(static_cast<int>(Status::FrameTooShort),
                    static_cast<int>(FrameCodec::tryDecode(
                        frame.data(), frame.size() - 1, out)));
  // Buffer below the minimum header.
  TEST_ASSERT_EQUAL(static_cast<int>(Status::FrameTooShort),
                    static_cast<int>(FrameCodec::tryDecode(
                        frame.data(), 3, out)));
}

void test_decode_rejects_header_checksum_mismatch(void) {
  auto frame = makeFrame(0x0001, 0x0001, {0xAA});
  frame[7] ^= 0xFF;  // flip all bits of head_cksum
  DecodedFrame out{};
  TEST_ASSERT_EQUAL(static_cast<int>(Status::ChecksumError),
                    static_cast<int>(FrameCodec::tryDecode(
                        frame.data(), frame.size(), out)));
}

void test_decode_rejects_data_checksum_mismatch(void) {
  auto frame = makeFrame(0x0001, 0x0001, {0xAA, 0xBB});
  frame.back() ^= 0xFF;  // flip data_cksum
  DecodedFrame out{};
  TEST_ASSERT_EQUAL(static_cast<int>(Status::ChecksumError),
                    static_cast<int>(FrameCodec::tryDecode(
                        frame.data(), frame.size(), out)));
}

void test_encode_rejects_oversized_payload(void) {
  std::vector<uint8_t> payload(FrameCodec::kMaxDataLen + 1, 0x00);
  std::vector<uint8_t> buf(1024);
  size_t written = 0;
  TEST_ASSERT_EQUAL(static_cast<int>(Status::FrameTooLong),
                    static_cast<int>(FrameCodec::encode(
                        1, 2, payload.data(), payload.size(), buf.data(),
                        buf.size(), written)));
}

void test_encode_rejects_tiny_output_buffer(void) {
  uint8_t out[4];
  size_t written = 0;
  TEST_ASSERT_EQUAL(static_cast<int>(Status::FrameTooLong),
                    static_cast<int>(FrameCodec::encode(1, 2, nullptr, 0, out,
                                                        sizeof(out), written)));
}

void test_expected_length_from_partial_header(void) {
  const uint8_t partial_header[] = {0x01, 0x00, 0x00, 0x00, 0x08};
  // len high=0x00, len low=0x08 -> 8 bytes payload
  TEST_ASSERT_EQUAL_size_t(FrameCodec::kHeaderSize + 8 +
                               FrameCodec::kDataChecksum,
                           FrameCodec::expectedLength(partial_header, 5));

  // Below header size — returns lower bound.
  TEST_ASSERT_EQUAL_size_t(FrameCodec::kMinFrameSize,
                           FrameCodec::expectedLength(nullptr, 0));
}

void test_little_endian_helpers_roundtrip(void) {
  uint8_t buf[4] = {0};

  FrameCodec::writeU32LE(0xDEADBEEFu, buf);
  TEST_ASSERT_EQUAL_HEX8(0xEF, buf[0]);
  TEST_ASSERT_EQUAL_HEX8(0xBE, buf[1]);
  TEST_ASSERT_EQUAL_HEX8(0xAD, buf[2]);
  TEST_ASSERT_EQUAL_HEX8(0xDE, buf[3]);
  TEST_ASSERT_EQUAL_HEX32(0xDEADBEEFu, FrameCodec::readU32LE(buf));

  const float kPi = 3.14159f;
  FrameCodec::writeFloatLE(kPi, buf);
  float out = FrameCodec::readFloatLE(buf);
  TEST_ASSERT_FLOAT_WITHIN(1e-6f, kPi, out);
}

void test_zero_payload_roundtrip(void) {
  auto frame = makeFrame(0x0100, 0x7FFF, {});
  TEST_ASSERT_EQUAL_size_t(
      FrameCodec::kHeaderSize + FrameCodec::kDataChecksum, frame.size());

  DecodedFrame out{};
  TEST_ASSERT_EQUAL(static_cast<int>(Status::Ok),
                    static_cast<int>(FrameCodec::tryDecode(
                        frame.data(), frame.size(), out)));
  TEST_ASSERT_EQUAL_UINT16(0, out.data_len);
  TEST_ASSERT_EQUAL_UINT16(0x7FFF, out.type);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_checksum_is_xor_then_invert);
  RUN_TEST(test_encode_layout_is_stable);
  RUN_TEST(test_encode_decode_roundtrip);
  RUN_TEST(test_decode_rejects_bad_sof);
  RUN_TEST(test_decode_rejects_short_buffer);
  RUN_TEST(test_decode_rejects_header_checksum_mismatch);
  RUN_TEST(test_decode_rejects_data_checksum_mismatch);
  RUN_TEST(test_encode_rejects_oversized_payload);
  RUN_TEST(test_encode_rejects_tiny_output_buffer);
  RUN_TEST(test_expected_length_from_partial_header);
  RUN_TEST(test_little_endian_helpers_roundtrip);
  RUN_TEST(test_zero_payload_roundtrip);
  return UNITY_END();
}
