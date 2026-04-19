/*
 * onboard_ambient_light
 * ---------------------
 * Poll the BH1750 ambient-light sensor that ships on the Seeed mmWave module.
 * Prints one lux reading per available sample.
 *
 * Hardware:
 *   - Seeed XIAO ESP32-C6
 *   - Seeed mmWave module (onboard BH1750 on default I2C bus, ADDR tied low)
 *
 * Expected serial output:
 *   LUX: 238.33
 *   LUX: 241.67
 *   ...
 *
 * Dependency:
 *   hp_BH1750 (pulled in by extras/OnboardAmbientLight.h)
 */

#include <Arduino.h>
#include "extras/OnboardAmbientLight.h"

seeed::mmwave::OnboardAmbientLight light;

void setup() {
  Serial.begin(115200);
  if (!light.begin()) {
    Serial.println("No BH1750 sensor found!");
    while (true) {}
  }
}

void loop() {
  float lux;
  if (light.read(lux)) {
    Serial.print("LUX: ");
    Serial.println(lux);
  }
}
