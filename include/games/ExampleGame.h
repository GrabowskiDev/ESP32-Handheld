#pragma once

#include "../core/IGame.h"
#include <Arduino.h>

class ExampleGame : public IGame {
private:
    int playerX;
    int playerY;
    int playerSpeed;
    uint32_t playerColor;

public:
    void start() override;
    void update() override;
    void draw() override;
    void stop() override;
};
