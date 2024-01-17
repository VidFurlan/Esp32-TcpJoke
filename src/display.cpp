#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initDisplay () {
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(2000);
  display.clearDisplay();
}

void displayText (String text, int curosr_x, int curosr_y, bool inversed_colors) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(curosr_x, curosr_y);

  display.println(text);
  display.display(); 
}

void terminalTextDisplay (String text, int curosr_x, int curosr_y, bool inversed_colors) {
  String text_to_display = "> ";
  for (int i = 0; i < text.length(); i++) {
    text_to_display += text.charAt(i);
    //Serial.print(text_to_display);
    displayText(text_to_display, curosr_x, curosr_y, inversed_colors); 
    delay(101);
  }
}