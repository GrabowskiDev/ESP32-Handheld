#pragma once

#include "../core/IGame.h"
#include <Arduino.h>

#define TETRIS_ROWS 20
#define TETRIS_COLS 10
#define BLOCK_SIZE 10

class TetrisGame : public IGame {
private:
    uint32_t board[TETRIS_ROWS][TETRIS_COLS];
    
    int currentPiece;
    int currentRotation;
    int pieceX;
    int pieceY;

    uint32_t score;
    bool gameOver;
    unsigned long lastFallTime;
    unsigned long fallSpeed;
    int heldPiece;
    bool hasHeldThisTurn;
    unsigned long lastLeftMoveTime;
    unsigned long lastRightMoveTime;

    void spawnPiece();
    bool checkCollision(int pX, int pY, int rot);
    void lockPiece();
    void clearLines();
    void resetBoard();
    void drawBlock(int x, int y, uint32_t color);
    void drawGhostBlock(int x, int y, uint32_t color);

public:
    void start() override;
    void update() override;
    void draw() override;
    void stop() override;
};
