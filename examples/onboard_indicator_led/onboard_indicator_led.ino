/*
 * onboard_indicator_led
 * ---------------------
 * Drive the onboard WS2812 indicator LED that ships on the Seeed mmWave module.
 * Blinks red 10 times, then fades red to black.
 *
 * Hardware:
 *   - Seeed XIAO ESP32-C6
 *   - Seeed mmWave module (onboard WS2812 on D1)
 *
 * Dependency:
 *   Adafruit NeoPixel (pulled in by extras/OnboardIndicator.h)
 */

#include <Arduino.h>
#include "extras/OnboardIndicator.h"

seeed::mmwave::OnboardIndicator indicator;

void setup() {
  Serial.begin(115200);
  indicator.begin(255);  // full brightness
}

void loop() {
  for (int i = 0; i < 10; i++) {
    indicator.set(255, 0, 0);
    delay(100);
    indicator.off();
    delay(100);
  }
  for (int i = 255; i >= 0; i--) {
    indicator.set(i, 0, 0);
    delay(10);
  }
}
