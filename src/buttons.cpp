#include "buttons.h"

namespace Buttons {

    struct ButtonState {
        int pin;
        bool isPressed;
        bool wasPressed;
        unsigned long lastDebounceTime;
    };

    static ButtonState buttons[BTN_COUNT] = {
        {BTN_UP_PIN, false, false, 0},
        {BTN_DOWN_PIN, false, false, 0},
        {BTN_LEFT_PIN, false, false, 0},
        {BTN_RIGHT_PIN, false, false, 0},
        {BTN_A_PIN, false, false, 0},
        {BTN_B_PIN, false, false, 0},
        {BTN_START_PIN, false, false, 0},
        {BTN_SELECT_PIN, false, false, 0}
    };

    const unsigned long DEBOUNCE_DELAY = 15; // milliseconds

    void setup() {
        for (int i = 0; i < BTN_COUNT; i++) {
            pinMode(buttons[i].pin, INPUT_PULLUP);
        }
    }

    void update() {
        unsigned long currentMillis = millis();

        for (int i = 0; i < BTN_COUNT; i++) {
            buttons[i].wasPressed = buttons[i].isPressed;

            // Read state (LOW means pressed because of INPUT_PULLUP)
            bool reading = (digitalRead(buttons[i].pin) == LOW);

            // Basic debouncing
            if (reading != buttons[i].isPressed) {
                if ((currentMillis - buttons[i].lastDebounceTime) > DEBOUNCE_DELAY) {
                    buttons[i].isPressed = reading;
                    buttons[i].lastDebounceTime = currentMillis;
                }
            } else {
                buttons[i].lastDebounceTime = currentMillis;
            }
        }
    }

    bool isPressed(ButtonID btn) {
        if (btn < 0 || btn >= BTN_COUNT) return false;
        return buttons[btn].isPressed;
    }

    bool justPressed(ButtonID btn) {
        if (btn < 0 || btn >= BTN_COUNT) return false;
        return buttons[btn].isPressed && !buttons[btn].wasPressed;
    }

    bool justReleased(ButtonID btn) {
        if (btn < 0 || btn >= BTN_COUNT) return false;
        return !buttons[btn].isPressed && buttons[btn].wasPressed;
    }

}
