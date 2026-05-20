#pragma once

// IGame is a pure virtual interface that all future games must implement.
class IGame {
public:
    virtual ~IGame() {}
    virtual bool isMainMenu() const { return false; }

    // Called once when the game is loaded into the GameManager
    virtual void start() = 0;

    // Called every frame to handle player input and internal game logic
    virtual void update() = 0;

    // Called every frame to render graphics to the screen (backbuffer)
    virtual void draw() = 0;

    // Called once when the game is unloaded or swapped out
    virtual void stop() = 0;
};
