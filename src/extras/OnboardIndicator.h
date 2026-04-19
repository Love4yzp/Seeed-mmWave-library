/**
 * @file OnboardIndicator.h
 * @brief Thin wrapper around the onboard WS2812 RGB indicator LED.
 *
 * The Seeed mmWave module carries a single WS2812 RGB LED wired to D1 on the
 * XIAO ESP32 carrier. This header is OPTIONAL — including it is what pulls in
 * the Adafruit_NeoPixel dependency. The core mmWave driver does not require it.
 *
 * @copyright © 2026, Seeed Studio
 */

#ifndef SEEED_MMWAVE_EXTRAS_ONBOARD_INDICATOR_H
#define SEEED_MMWAVE_EXTRAS_ONBOARD_INDICATOR_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <stdint.h>

namespace seeed {
namespace mmwave {

class OnboardIndicator {
 public:
  static constexpr uint8_t kDefaultPin        = D1;
  static constexpr uint8_t kDefaultBrightness = 8;

  explicit OnboardIndicator(uint8_t pin = kDefaultPin)
      : _pixels(1, pin, NEO_GRB + NEO_KHZ800) {}

  void begin(uint8_t brightness = kDefaultBrightness) {
    _pixels.begin();
    _pixels.setBrightness(brightness);
    _pixels.clear();
    _pixels.show();
  }

  void set(uint8_t r, uint8_t g, uint8_t b) {
    _pixels.setPixelColor(0, _pixels.Color(r, g, b));
    _pixels.show();
  }

  void off() { set(0, 0, 0); }

  void setBrightness(uint8_t brightness) { _pixels.setBrightness(brightness); }

  Adafruit_NeoPixel& raw() { return _pixels; }

 private:
  Adafruit_NeoPixel _pixels;
};

}  // namespace mmwave
}  // namespace seeed

#endif  // SEEED_MMWAVE_EXTRAS_ONBOARD_INDICATOR_H
