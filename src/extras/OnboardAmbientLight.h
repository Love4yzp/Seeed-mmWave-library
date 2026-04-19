/**
 * @file OnboardAmbientLight.h
 * @brief Thin wrapper around the onboard BH1750 ambient-light sensor.
 *
 * The Seeed mmWave module carries a BH1750 ambient-light sensor on the default
 * I2C bus (ADDR pin tied to ground). This header is OPTIONAL — including it is
 * what pulls in the hp_BH1750 dependency. The core mmWave driver does not
 * require it.
 *
 * @copyright © 2026, Seeed Studio
 */

#ifndef SEEED_MMWAVE_EXTRAS_ONBOARD_AMBIENT_LIGHT_H
#define SEEED_MMWAVE_EXTRAS_ONBOARD_AMBIENT_LIGHT_H

#include <Arduino.h>
#include <hp_BH1750.h>
#include <stdint.h>

namespace seeed {
namespace mmwave {

class OnboardAmbientLight {
 public:
  static constexpr uint8_t kDefaultMtreg = 254;

  bool begin(uint8_t quality = BH1750_QUALITY_HIGH2,
             uint8_t mtreg   = kDefaultMtreg) {
    _quality = quality;
    _mtreg   = mtreg;
    if (!_bh1750.begin(BH1750_TO_GROUND)) {
      return false;
    }
    _bh1750.calibrateTiming();
    _bh1750.start(_quality, _mtreg);
    return true;
  }

  // Non-blocking: returns true when a fresh sample was consumed and `lux` updated.
  // Internally restarts the next measurement so the caller just polls.
  bool read(float& lux) {
    if (!_bh1750.hasValue()) {
      return false;
    }
    lux = _bh1750.getLux();
    _bh1750.start(_quality, _mtreg);
    return true;
  }

  // Latest cached value (may be stale; use read() to refresh).
  float lastLux() const { return _last; }

  // Convenience: block until a value is available or timeout elapses.
  bool readBlocking(float& lux, uint32_t timeout_ms = 1000) {
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
      if (read(lux)) {
        _last = lux;
        return true;
      }
      delay(5);
    }
    return false;
  }

  hp_BH1750& raw() { return _bh1750; }

 private:
  hp_BH1750 _bh1750;
  uint8_t _quality = BH1750_QUALITY_HIGH2;
  uint8_t _mtreg   = kDefaultMtreg;
  float _last      = 0.0f;
};

}  // namespace mmwave
}  // namespace seeed

#endif  // SEEED_MMWAVE_EXTRAS_ONBOARD_AMBIENT_LIGHT_H
