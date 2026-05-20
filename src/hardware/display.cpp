#include "hardware/display.h"

namespace Display {

    TFT_eSPI tft = TFT_eSPI(); 
    TFT_eSprite backbuffer = TFT_eSprite(&tft);

    // GPIO 8 controls display backlight
    const uint8_t BACKLIGHT_PIN = 8;
    static uint8_t currentBrightness = 100; // Percentage: 10 to 100

    void setup() {
        // Initialize backlight pin first to avoid flashing
        pinMode(BACKLIGHT_PIN, OUTPUT);
        uint8_t pwmValue = (currentBrightness * 255) / 100;
        analogWrite(BACKLIGHT_PIN, pwmValue);

        tft.init();
        tft.setRotation(3); // Landscape mode
        
        // Try creating a full-screen backbuffer for games to avoid flicker.
        // Screen is typically 320x240 for ILI9341. (Size: ~153KB RAM)
        // If this crashes due to memory, we'll need to reduce size or not use double-buffering.
        void* ptr = backbuffer.createSprite(tft.width(), tft.height());
        
        if (!ptr) {
            Serial.println("Display setup Warning: Not enough RAM for full screen double buffering!");
        } else {
            Serial.println("Display initialized with full double-buffering Sprite.");
            backbuffer.fillSprite(TFT_BLACK);
            backbuffer.setTextSize(2);
            backbuffer.setTextColor(TFT_WHITE, TFT_BLACK);
        }
    }

    void clear(uint32_t color) {
        // Clear the backbuffer instead of screen
        backbuffer.fillSprite(color);
    }

    void update() {
        // Push the backbuffer to the physical screen
        backbuffer.pushSprite(0, 0);
    }

    void setBrightness(uint8_t percent) {
        if (percent < 10) percent = 10;
        if (percent > 100) percent = 100;
        currentBrightness = percent;
        
        // Map 10-100% to 25-255 PWM duty cycle
        uint8_t pwmValue = (percent * 255) / 100;
        analogWrite(BACKLIGHT_PIN, pwmValue);
    }

    uint8_t getBrightness() {
        return currentBrightness;
    }

}
