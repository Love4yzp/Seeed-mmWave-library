#include <Arduino.h>
#include <U8x8lib.h>
#include <Wire.h>

/**
 * @brief Create an instance of the U8X8_SSD1306_128X64_NONAME_SW_I2C class,
 * which is used to communicate with the OLED display
 * @note the Grove port is driven by D0, D10
 */
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock(SCL)=*/D0, /* data(SDA)=*/D10,
                                       /* reset=*/U8X8_PIN_NONE);

void setup(void) {
  // Initialize the OLED display
  u8x8.begin();

  // Set the flip mode of the display to 1, which rotates the screen 180 degrees
  u8x8.setFlipMode(1);
}

void loop(void) {
  // Set the font of the display to chroma48medium8_r
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  // Set the cursor position to (0, 0)
  u8x8.setCursor(0, 0);

  // Print the string "Hello World!" to the display
  u8x8.print("Hello World!");
}