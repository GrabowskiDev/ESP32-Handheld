#include "games/ExampleGame.h"
#include "hardware/buttons.h"
#include "hardware/display.h"
#include "hardware/sound.h"

void ExampleGame::start() {
    // Reset player state when game starts
    playerX = 160;
    playerY = 120;
    playerSpeed = 3;
    playerColor = TFT_GREEN;

    Serial.println("ExampleGame: Started!");
    // You could load game-specific sounds or variables here
}

void ExampleGame::update() {
    // Handle Input
    if (Buttons::isPressed(BTN_UP)) playerY -= playerSpeed;
    if (Buttons::isPressed(BTN_DOWN)) playerY += playerSpeed;
    if (Buttons::isPressed(BTN_LEFT)) playerX -= playerSpeed;
    if (Buttons::isPressed(BTN_RIGHT)) playerX += playerSpeed;

    if (Buttons::justPressed(BTN_A)) {
        playerColor = TFT_RED;
        Sound::playSFX("/move.wav"); 
        Serial.println("ExampleGame: Action A!");
    }
    
    if (Buttons::justReleased(BTN_A)) {
        playerColor = TFT_GREEN;
    }

    if (Buttons::justPressed(BTN_B)) {
        Sound::playSFX("/impact.wav"); 
        Serial.println("ExampleGame: Action B!");
    }

    if (Buttons::justPressed(BTN_START)) {
        Serial.println("ExampleGame: Start Pressed!");
        // Example: GameManager->switchGame(new MainMenu());
    }

    if (Buttons::justPressed(BTN_SELECT)) {
        Serial.println("ExampleGame: Select Pressed!");
    }

    // Boundaries
    if (playerX < 10) playerX = 10;
    if (playerX > 310) playerX = 310;
    if (playerY < 10) playerY = 10;
    if (playerY > 230) playerY = 230;
}

void ExampleGame::draw() {
    // Fill the screen/backbuffer with black
    Display::clear(TFT_BLACK);

    // Draw HUD
    Display::backbuffer.setCursor(10, 10);
    Display::backbuffer.setTextColor(TFT_WHITE, TFT_BLACK);
    Display::backbuffer.setTextSize(1);
    Display::backbuffer.println("ESP32 Handheld OS");
    
    Display::backbuffer.setCursor(10, 25);
    Display::backbuffer.printf("X: %d Y: %d", playerX, playerY);
    
    Display::backbuffer.setCursor(10, 220);
    Display::backbuffer.println("Example Game Active");

    // Draw Player
    Display::backbuffer.fillCircle(playerX, playerY, 10, playerColor);
}

void ExampleGame::stop() {
    Serial.println("ExampleGame: Stopped/Cleaned up!");
    // Unload any large specific things if we had them
}
