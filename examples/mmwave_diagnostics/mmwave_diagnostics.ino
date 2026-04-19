#include <Arduino.h>

// Define the serial communication object for the mmWave sensor
#ifdef ESP32
// On ESP32, use HardwareSerial library to create a serial object
#  include <HardwareSerial.h>
HardwareSerial mmwaveSerial(0);
#else
// On other boards, use Serial1 as the serial object
#  define mmwaveSerial Serial1
#endif

void setup() {
  // Initialize the serial communication for the mmWave sensor at 115200 baud
  mmwaveSerial.begin(115200);
  // Initialize the serial communication for the PC serial monitor at 115200
  // baud
  Serial.begin(115200);
}

void loop() {
  // Check if there is incoming data from the mmWave sensor
  if (mmwaveSerial.available()) {
    // Read the incoming data from the mmWave sensor
    uint8_t data = mmwaveSerial.read();
    // Print the data in hexadecimal format to the serial monitor
    Serial.println(data, HEX);
  }
  // Check if there is incoming data from the PC serial monitor
  if (Serial.available()) {
    // Read the incoming data from the serial monitor
    // and write it to the mmWave sensor
    mmwaveSerial.write(Serial.read());
  }
}