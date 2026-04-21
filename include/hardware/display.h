#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

namespace Display {
    // Expose the main display object and the backbuffer sprite
    extern TFT_eSPI tft;
    extern TFT_eSprite backbuffer;

    void setup();
    void clear(uint32_t color = TFT_BLACK);
    void update(); // Pushes backbuffer to screen
}
