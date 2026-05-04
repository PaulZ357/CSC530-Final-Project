#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  // ---- LOCK ICON ----

  // Lock body
  display.fillRect(50, 30, 28, 20, SSD1306_WHITE);

  // Lock shackle (top curve)
  display.drawCircle(64, 30, 10, SSD1306_WHITE);

  // Cut inside of circle to make it look like a U-shape
  display.fillRect(54, 30, 20, 10, SSD1306_BLACK);

  // Optional keyhole
  display.fillCircle(64, 40, 2, SSD1306_BLACK);

  display.display();
}

void loop() {
}