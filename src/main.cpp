#include <Arduino.h>
#include "core/config.h"
#include "hardware/display.h"
#include "hardware/buttons.h"
#include "hardware/sound.h"
#include "core/GameManager.h"
#include "games/MainMenu.h"
#include "games/ExampleGame.h"
#include "games/TetrisGame.h"

// The core engine that manages states/games
GameManager gameManager;

void setup() {
    Serial.begin(115200);
    delay(1000); // Wait for serial monitor to connect

    Serial.println("Starting Game Console Framework...");

    // Initialize all hardware abstractions
    Display::setup();
    Buttons::setup();
    Sound::setup();
    Sound::startTask();

    Serial.println("Framework Hardware Ready. Loading Main Menu.");

    // Load the Main Menu launcher into the Game Manager!
    gameManager.switchGame(new MainMenu());
}

void loop() {
    // 1. Update global hardware
    Buttons::update();

    // --- GLOBAL EXIT CHORD ---
    // Pressing SELECT + START together returns the player to the Main Menu (if not already there)
    if (gameManager.getCurrentGame() && !gameManager.getCurrentGame()->isMainMenu()) {
        if (Buttons::isPressed(BTN_SELECT) && Buttons::isPressed(BTN_START)) {
            Serial.println("Shortcut triggered: Returning to Main Menu.");
            Sound::playSFX("/extinguish.wav");
            gameManager.switchGame(new MainMenu());
            delay(300); // Debounce to prevent immediate double-triggers
        }
    }

    // 2. Pass control to the actively running game
    gameManager.update();

    // 3. Have the active game render itself, then push it to screen
    gameManager.draw();
    Display::update();

    // 4. Frame Delay (approx 60 frames per second limit)
    delay(16); 
}

