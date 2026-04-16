#pragma once

#include <Arduino.h>
#include "config.h"

enum ButtonID {
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_A,
    BTN_B,
    BTN_START,
    BTN_SELECT,
    BTN_COUNT // Keep this last
};

namespace Buttons {
    void setup();
    void update(); // Call this in loop()

    bool isPressed(ButtonID btn);
    bool justPressed(ButtonID btn);
    bool justReleased(ButtonID btn);
}
