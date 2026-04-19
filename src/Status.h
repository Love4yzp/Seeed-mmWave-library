/**
 * @file Status.h
 * @brief Status codes returned by the Seeed mmWave v2 API.
 *
 * Replaces the older "return bool" convention on getters and configuration
 * methods, which could not distinguish "no data yet" from "transport error"
 * from "checksum failure" etc. Kept header-only and Arduino-free so the
 * FrameCodec layer can be unit-tested on the host.
 *
 * @copyright © 2026, Seeed Studio
 */

#ifndef SEEED_ARDUINO_MMWAVE_STATUS_H
#define SEEED_ARDUINO_MMWAVE_STATUS_H

#include <cstdint>

namespace seeed {
namespace mmwave {

enum class Status : uint8_t {
  Ok,               // Operation succeeded.
  NoData,           // Requested datum has not arrived since the last read.
  Timeout,          // Deadline elapsed waiting for data.
  ChecksumError,    // Header or data checksum mismatch.
  FrameTooShort,    // Buffer shorter than the minimum frame size.
  FrameTooLong,     // LEN field exceeds the supported frame buffer.
  BadSof,           // First byte is not the Start-of-Frame marker.
  UnsupportedType,  // Frame type not handled by this device driver.
  Busy,             // Another operation is in progress (e.g. pending command).
  IoError,          // Lower-level serial/transport failure.
};

inline const char* statusName(Status s) {
  switch (s) {
    case Status::Ok:               return "Ok";
    case Status::NoData:           return "NoData";
    case Status::Timeout:          return "Timeout";
    case Status::ChecksumError:    return "ChecksumError";
    case Status::FrameTooShort:    return "FrameTooShort";
    case Status::FrameTooLong:     return "FrameTooLong";
    case Status::BadSof:           return "BadSof";
    case Status::UnsupportedType:  return "UnsupportedType";
    case Status::Busy:             return "Busy";
    case Status::IoError:          return "IoError";
  }
  return "?";
}

}  // namespace mmwave
}  // namespace seeed

#endif  // SEEED_ARDUINO_MMWAVE_STATUS_H
