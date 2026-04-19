/**
 * @file FrameCodec.h
 * @brief Pure encode/decode of the Seeed mmWave UART frame format.
 *
 * Frame layout:
 *   +------+------+------+------+-------------+-------+-------------+
 *   | SOF  | ID   | LEN  | TYPE | HEAD_CKSUM  | DATA  | DATA_CKSUM  |
 *   | 1 B  | 2 B  | 2 B  | 2 B  | 1 B         | LEN B | 1 B         |
 *   +------+------+------+------+-------------+-------+-------------+
 *
 *   SOF         = 0x01
 *   ID, LEN, TYPE are big-endian 16-bit.
 *   HEAD_CKSUM  = ~XOR of the seven header bytes that precede it.
 *   DATA_CKSUM  = ~XOR of the DATA bytes.
 *   DATA payloads are little-endian.
 *
 * This header deliberately avoids including `Arduino.h` so it can be compiled
 * and unit-tested on the host (PlatformIO native + Unity / GoogleTest).
 *
 * @copyright © 2026, Seeed Studio
 */

#ifndef SEEED_ARDUINO_MMWAVE_FRAME_CODEC_H
#define SEEED_ARDUINO_MMWAVE_FRAME_CODEC_H

#include <cstddef>
#include <cstdint>

#include "Status.h"

namespace seeed {
namespace mmwave {

struct DecodedFrame {
  uint16_t id;
  uint16_t type;
  const uint8_t* data;  // Points into the caller-provided buffer.
  uint16_t data_len;
};

struct FrameCodec {
  static constexpr uint8_t kSofByte       = 0x01;
  static constexpr size_t kHeaderSize     = 8;  // SOF + ID(2) + LEN(2) + TYPE(2) + HEAD_CKSUM(1)
  static constexpr size_t kDataChecksum   = 1;
  static constexpr size_t kMinFrameSize   = kHeaderSize + kDataChecksum;  // empty payload
  static constexpr size_t kMaxDataLen     = 512;  // FRAME_BUFFER_SIZE

  // XOR-then-invert checksum used by both the header and data sections.
  static uint8_t checksum(const uint8_t* data, size_t len);

  // Given the first few bytes of a frame-in-progress, return the total byte
  // count the complete frame will have. If the buffer is shorter than the
  // header, returns kMinFrameSize as a lower bound.
  static size_t expectedLength(const uint8_t* buf, size_t len);

  // Attempt to decode a single frame at the start of `buf`.
  //
  // Returns:
  //   Ok              - `out` is populated; `out.data` points into `buf`.
  //   FrameTooShort   - `len` is smaller than required.
  //   BadSof          - first byte is not kSofByte.
  //   FrameTooLong    - LEN field exceeds kMaxDataLen.
  //   ChecksumError   - header or data checksum failed.
  static Status tryDecode(const uint8_t* buf, size_t len, DecodedFrame& out);

  // Encode a frame into a caller-owned buffer.
  //
  // Required `out_cap` is kHeaderSize + data_len + kDataChecksum. On success
  // `out_len` receives the number of bytes written.
  //
  // Returns:
  //   Ok            - `out_len` written.
  //   FrameTooLong  - `data_len` > kMaxDataLen or `out_cap` too small.
  static Status encode(uint16_t id, uint16_t type,
                       const uint8_t* data, size_t data_len,
                       uint8_t* out, size_t out_cap, size_t& out_len);

  // Little-endian byte helpers for common payload scalars.
  static float    readFloatLE(const uint8_t* p);
  static uint32_t readU32LE(const uint8_t* p);
  static void     writeFloatLE(float v, uint8_t* p);
  static void     writeU32LE(uint32_t v, uint8_t* p);
};

}  // namespace mmwave
}  // namespace seeed

#endif  // SEEED_ARDUINO_MMWAVE_FRAME_CODEC_H
