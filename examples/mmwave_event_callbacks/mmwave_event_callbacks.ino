/*
 * mmwave_event_callbacks
 * ----------------------
 * Subscribe to individual events instead of polling N getters. Each
 * callback fires from inside `update()` whenever the corresponding frame
 * arrives and parses successfully.
 *
 * Hardware:
 *   - Seeed XIAO ESP32-C6
 *   - Seeed MR60BHA2 60GHz mmWave breathing / heartbeat module
 *
 * The polling API (`getBreathRate`, `getHeartRate`, …) still works and can
 * be mixed with callbacks. The two paths see the same data.
 *
 * Callback contract:
 *   - Runs on the task that called `update()`.
 *   - Keep work short: no long-running logic, no blocking I/O, no
 *     `update()` recursion.
 */

#include <Arduino.h>
#include "Seeed_Arduino_mmWave.h"

#ifdef ESP32
#  include <HardwareSerial.h>
HardwareSerial mmWaveSerial(0);
#else
#  define mmWaveSerial Serial1
#endif

SEEED_MR60BHA2 mmWave;

void setup() {
  Serial.begin(115200);
  mmWave.begin(&mmWaveSerial);

  mmWave.onBreathRate([](float rate) {
    Serial.printf("[event] breath rate: %.2f\n", rate);
  });

  mmWave.onHeartRate([](float rate) {
    Serial.printf("[event] heart rate:  %.2f\n", rate);
  });

  mmWave.onDistance([](float distance) {
    Serial.printf("[event] distance:    %.2f\n", distance);
  });

  mmWave.onPresence([](bool present) {
    Serial.printf("[event] presence:    %s\n", present ? "yes" : "no");
  });

  mmWave.onError([](seeed::mmwave::Status status) {
    Serial.printf("[error] %s\n", seeed::mmwave::statusName(status));
  });

  Serial.println("Subscribed. Waiting for frames...");
}

void loop() {
  mmWave.update(100);

  // Print running stats once a second so you can watch parse health at a glance.
  static uint32_t next_print = 0;
  if (millis() >= next_print) {
    next_print = millis() + 1000;
    const auto& s = mmWave.stats();
    Serial.printf("stats: rx=%u cksumErr=%u dropped=%u bytes=%u\n",
                  s.framesRx, s.checksumErrors, s.droppedByQueueFull,
                  s.bytesRx);
  }
}
