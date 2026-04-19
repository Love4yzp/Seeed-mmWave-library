/*
 * integration_breath_monitor
 * --------------------------
 * End-to-end demo: MR60BHA2 breathing & heart rate + OLED display + onboard
 * WS2812 indicator (green when heart rate >= 75 bpm, red otherwise).
 *
 * Hardware:
 *   - Seeed XIAO ESP32-C6
 *   - Seeed MR60BHA2 60GHz mmWave module (with onboard WS2812 on D1)
 *   - Grove SSD1306 128x64 OLED on D0 (SCL) / D10 (SDA)
 *
 * Dependencies (only for this integration example):
 *   - U8g2 (U8x8 bundled)
 *   - Adafruit NeoPixel (pulled in by extras/OnboardIndicator.h)
 */

#include <Arduino.h>
#include <U8x8lib.h>
#include "Seeed_Arduino_mmWave.h"
#include "extras/OnboardIndicator.h"

#ifdef ESP32
#  include <HardwareSerial.h>
HardwareSerial mmWaveSerial(0);
#else
#  define mmWaveSerial Serial1
#endif

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(
    /* clock=*/D0, /* data=*/D10,
    /* reset=*/U8X8_PIN_NONE);

seeed::mmwave::OnboardIndicator indicator;
SEEED_MR60BHA2 mmWave;

typedef enum { LABEL_BREATH, LABEL_HEART, LABEL_DISTANCE } Label;

static const char* TAG_Breath   = "BreathRate";
static const char* TAG_Heart    = "HeartRate";
static const char* TAG_Distance = "Distance";

void updateDisplay(Label label, float value) {
  static float last_breath_rate = -1.0;
  static float last_heart_rate  = -1.0;
  static float last_distance    = -1.0;
  switch (label) {
    case LABEL_BREATH:
      if (value == last_breath_rate) break;
      u8x8.setCursor(11, 3);
      u8x8.print(value);
      last_breath_rate = value;
      break;
    case LABEL_HEART:
      if (value == last_heart_rate) break;
      u8x8.setCursor(11, 5);
      u8x8.print(value);
      last_heart_rate = value;
      break;
    case LABEL_DISTANCE:
      if (value == last_distance) break;
      u8x8.setCursor(11, 7);
      u8x8.print(value);
      last_distance = value;
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  mmWave.begin(&mmWaveSerial);
  Serial.println("Welcome, my heart is beatin'");

  indicator.begin();
  indicator.set(0, 125, 0);  // green idle

  u8x8.begin();
  u8x8.setFlipMode(3);
  u8x8.clearDisplay();
  u8x8.setFont(u8x8_font_victoriamedium8_r);
  u8x8.setCursor(1, 0);
  u8x8.print("Rate & Distance");
  u8x8.setCursor(0, 3);
  u8x8.print(TAG_Breath);
  u8x8.setCursor(0, 5);
  u8x8.print(TAG_Heart);
  u8x8.setCursor(0, 7);
  u8x8.print(TAG_Distance);
  u8x8.setFont(u8x8_font_chroma48medium8_n);
}

void loop() {
  if (!mmWave.update(100)) return;

  float breath_rate = 0;
  float heart_rate  = 0;
  float distance    = 0;

  if (mmWave.getBreathRate(breath_rate)) {
    updateDisplay(LABEL_BREATH, breath_rate);
  }

  if (mmWave.getHeartRate(heart_rate)) {
    if (heart_rate < 75) {
      indicator.set(125, 0, 0);  // red: low
    } else {
      indicator.set(0, 125, 0);  // green: normal
    }
    updateDisplay(LABEL_HEART, heart_rate);
  }

  if (mmWave.getDistance(distance)) {
    updateDisplay(LABEL_DISTANCE, distance);
    if ((70 - distance) < 0) {
      Serial.println("No one here");
    }
  }

  Serial.printf("breath_rate: %.2f\n", breath_rate);
  Serial.printf("heart_rate : %.2f\n", heart_rate);
  Serial.printf("distance   : %.2f\n", distance);
}
