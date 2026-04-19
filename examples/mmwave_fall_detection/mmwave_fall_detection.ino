/*
 * mmwave_fall_detection
 * ---------------------
 * Minimal example: print human-presence and fall-detection state from a
 * Seeed MR60FDA2 60GHz mmWave fall-detection sensor.
 *
 * Hardware:
 *   - Seeed XIAO ESP32-C6 (or other XIAO ESP32 series)
 *   - Seeed MR60FDA2 60GHz mmWave fall-detection module
 *
 * Wiring (carrier board; default factory combo):
 *   XIAO  <-- UART -->  MR60FDA2 (on-board, 115200 baud)
 *
 * Expected serial output (once a person is in range):
 *   human: 1    fall: 0
 *   human: 1    fall: 1   <-- a fall event
 *
 * See also examples/mmwave_event_callbacks for the event-driven API planned
 * for v2.0.
 */

#include <Arduino.h>
#include "Seeed_Arduino_mmWave.h"

#ifdef ESP32
#  include <HardwareSerial.h>
HardwareSerial mmWaveSerial(0);
#else
#  define mmWaveSerial Serial1
#endif

SEEED_MR60FDA2 mmWave;

void setup() {
  Serial.begin(115200);
  mmWave.begin(&mmWaveSerial);
  Serial.println("MR60FDA2 fall-detection ready");
}

void loop() {
  if (!mmWave.update(100)) {
    return;
  }

  bool is_human = false;
  bool is_fall  = false;
  bool have_human = mmWave.getHuman(is_human);
  bool have_fall  = mmWave.getFall(is_fall);

  if (have_human || have_fall) {
    Serial.printf("human: %d\tfall: %d\n", is_human ? 1 : 0, is_fall ? 1 : 0);
  }
}
