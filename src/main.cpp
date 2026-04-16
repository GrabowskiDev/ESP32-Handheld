#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "buttons.h"
#include "sound.h"
#include "GameManager.h"
#include "ExampleGame.h"

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

    // Background static music
    Sound::playBGM("/hum.wav", true);

    Serial.println("Framework Hardware Ready. Loading initial game.");

    // Load the first game into the Game Manager!
    gameManager.switchGame(new ExampleGame());
}

void loop() {
    // 1. Update global hardware
    Buttons::update();

    // 2. Pass control to the actively running game
    gameManager.update();

    // 3. Have the active game render itself, then push it to screen
    gameManager.draw();
    Display::update();

    // 4. Frame Delay (approx 60 frames per second limit)
    delay(16); 
}
