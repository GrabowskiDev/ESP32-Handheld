#include "core/GameManager.h"
#include <Arduino.h>

GameManager::GameManager() : currentGame(nullptr) {
}

GameManager::~GameManager() {
    if (currentGame) {
        currentGame->stop();
        delete currentGame;
    }
}

void GameManager::switchGame(IGame* newGame) {
    if (currentGame) {
        currentGame->stop();
        // Since we pass pointers, the GameManager takes ownership and cleans up
        delete currentGame; 
    }
    
    currentGame = newGame;
    
    if (currentGame) {
        currentGame->start();
    }
}

void GameManager::update() {
    if (currentGame) {
        currentGame->update();
    }
}

void GameManager::draw() {
    if (currentGame) {
        currentGame->draw();
    }
}
