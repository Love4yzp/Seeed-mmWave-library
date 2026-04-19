/*
 * integration_fall_alert
 * ----------------------
 * End-to-end demo: MR60FDA2 fall detection + onboard BH1750 ambient light
 * + onboard WS2812 indicator + a Grove relay that turns on a lamp in the dark
 * when a person is present or has fallen.
 *
 * Indicator colors:
 *   BLUE  - no presence
 *   GREEN - presence, no fall
 *   RED   - fall detected
 *
 * Hardware:
 *   - Seeed XIAO ESP32-C6
 *   - Seeed MR60FDA2 60GHz mmWave module
 *     (onboard BH1750 ambient light + onboard WS2812 on D1)
 *   - Grove relay module on D0 (Grove port SIG1)
 *
 * Dependencies (only for this integration example):
 *   - hp_BH1750  (pulled in by extras/OnboardAmbientLight.h)
 *   - Adafruit NeoPixel (pulled in by extras/OnboardIndicator.h)
 */

#include <Arduino.h>
#include "Seeed_Arduino_mmWave.h"
#include "extras/OnboardAmbientLight.h"
#include "extras/OnboardIndicator.h"

#ifdef ESP32
#  include <HardwareSerial.h>
HardwareSerial mmwaveSerial(0);
#else
#  define mmwaveSerial Serial1
#endif

#define LIGHT_GPIO D0
static const uint8_t dark_lux = 10;

SEEED_MR60FDA2 mmWave;
seeed::mmwave::OnboardIndicator indicator;
seeed::mmwave::OnboardAmbientLight light;

uint32_t sensitivity = 15;
float height = 2.8, threshold = 1.0;
float rect_XL, rect_XR, rect_ZF, rect_ZB;

typedef enum {
  EXIST_PEOPLE,
  NO_PEOPLE,
  PEOPLE_FALL,
} MMWAVE_STATUS;

MMWAVE_STATUS status = NO_PEOPLE, last_status = NO_PEOPLE;
float lux = 100;

void relay_init() { pinMode(LIGHT_GPIO, OUTPUT); }
void relay_on()   { digitalWrite(LIGHT_GPIO, HIGH); }
void relay_off()  { digitalWrite(LIGHT_GPIO, LOW); }

void setup() {
  Serial.begin(115200);
  mmWave.begin(&mmwaveSerial);
  relay_init();

  indicator.begin();
  indicator.set(125, 125, 125);

  if (!light.begin()) {
    Serial.println("BH1750 not found on onboard I2C");
  }

  mmWave.setUserLog(0);

  if (mmWave.setInstallationHeight(height)) {
    Serial.printf("setInstallationHeight success: %.2f\n", height);
  } else {
    Serial.println("setInstallationHeight failed");
  }
  if (mmWave.setThreshold(threshold)) {
    Serial.printf("setThreshold success: %.2f\n", threshold);
  } else {
    Serial.println("setThreshold failed");
  }
  if (mmWave.setSensitivity(sensitivity)) {
    Serial.printf("setSensitivity success %d\n", sensitivity);
  } else {
    Serial.println("setSensitivity failed");
  }

  if (mmWave.getRadarParameters(height, threshold, sensitivity, rect_XL,
                                rect_XR, rect_ZF, rect_ZB)) {
    Serial.printf("height: %.2f\tthreshold: %.2f\tsensitivity: %d\n", height,
                  threshold, sensitivity);
    Serial.printf(
        "rect_XL: %.2f\trect_XR: %.2f\trect_ZF: %.2f\trect_ZB: %.2f\n", rect_XL,
        rect_XR, rect_ZF, rect_ZB);
  } else {
    Serial.println("getRadarParameters failed");
  }
}

void loop() {
  if (mmWave.update(100)) {
    bool is_human, is_fall;
    if (mmWave.getHuman(is_human) && mmWave.getFall(is_fall)) {
      if (!is_human && !is_fall)        status = NO_PEOPLE;
      else if (is_fall)                 status = PEOPLE_FALL;
      else                              status = EXIST_PEOPLE;
    }
  }

  switch (status) {
    case NO_PEOPLE:    Serial.print("Waiting for people"); break;
    case EXIST_PEOPLE: Serial.print("PEOPLE !!!"); break;
    case PEOPLE_FALL:  Serial.print("FALL !!!"); break;
    default: break;
  }
  Serial.println();

  if (status != last_status) {
    switch (status) {
      case NO_PEOPLE:    indicator.set(0, 0, 255); break;
      case EXIST_PEOPLE: indicator.set(0, 255, 0); break;
      case PEOPLE_FALL:  indicator.set(255, 0, 0); break;
      default: break;
    }
    last_status = status;
  }

  float fresh_lux;
  if (light.read(fresh_lux)) lux = fresh_lux;
  Serial.printf("LUX: %.2f\t", lux);

  if ((status == EXIST_PEOPLE || status == PEOPLE_FALL) && lux < dark_lux) {
    relay_on();
  } else {
    relay_off();
  }
}
