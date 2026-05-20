#pragma once

#include "IGame.h"

// GameManager controls which game is currently running and distributes the engine ticks.
class GameManager {
private:
    IGame* currentGame;

public:
    GameManager();
    ~GameManager();

    // Swaps the active game. Unloads the old one and starts the new one.
    void switchGame(IGame* newGame);

    // Engine ticks to be called from main.cpp loop
    void update();
    void draw();
    
    IGame* getCurrentGame() const { return currentGame; }
};

extern GameManager gameManager;
