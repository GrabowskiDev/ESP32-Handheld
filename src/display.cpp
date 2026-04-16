#include "display.h"

namespace Display {

    TFT_eSPI tft = TFT_eSPI(); 
    TFT_eSprite backbuffer = TFT_eSprite(&tft);

    void setup() {
        tft.init();
        tft.setRotation(1); // Landscape mode
        
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

}
