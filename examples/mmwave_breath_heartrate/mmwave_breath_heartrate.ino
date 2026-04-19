/*
 * mmwave_breath_heartrate
 * -----------------------
 * Minimal example: print breathing rate, heart rate, distance and phase
 * readings from a Seeed MR60BHA2 60GHz mmWave sensor.
 *
 * Hardware:
 *   - Seeed XIAO ESP32-C6 (or other XIAO ESP32 series)
 *   - Seeed MR60BHA2 60GHz mmWave breathing / heartbeat module
 *
 * Wiring (carrier board; default factory combo):
 *   XIAO  <-- UART -->  MR60BHA2 (on-board, 115200 baud)
 *
 * Expected serial output (once a subject is in range):
 *   breath_rate: 16.50
 *   heart_rate:  72.00
 *   distance:    0.68
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
}

void loop() {
  if (mmWave.update(100)) {
    float total_phase, breath_phase, heart_phase;
    if (mmWave.getHeartBreathPhases(total_phase, breath_phase, heart_phase)) {
      Serial.printf("total_phase: %.2f\t", total_phase);
      Serial.printf("breath_phase: %.2f\t", breath_phase);
      Serial.printf("heart_phase: %.2f\n", heart_phase);
    }

    float breath_rate;
    if (mmWave.getBreathRate(breath_rate)) {
      Serial.printf("breath_rate: %.2f\n", breath_rate);
    }

    float heart_rate;
    if (mmWave.getHeartRate(heart_rate)) {
      Serial.printf("heart_rate: %.2f\n", heart_rate);
    }

    float distance;
    if (mmWave.getDistance(distance)) {
      Serial.printf("distance: %.2f\n", distance);
    }
  }
}