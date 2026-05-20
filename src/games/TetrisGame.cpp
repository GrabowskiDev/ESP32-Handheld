#include "games/TetrisGame.h"
#include "hardware/display.h"
#include "hardware/buttons.h"
#include "hardware/sound.h"

// 7 pieces, 4 rotations. Each 16-bit hex represents a 4x4 grid of blocks.
static const uint16_t SHAPES[7][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444}, // I
    {0x44C0, 0x8E00, 0x6440, 0x0E20}, // J
    {0x4460, 0x0E80, 0xC440, 0x2E00}, // L
    {0xCC00, 0xCC00, 0xCC00, 0xCC00}, // O
    {0x06C0, 0x8C40, 0x6C00, 0x4620}, // S
    {0x0E40, 0x4C40, 0x4E00, 0x4640}, // T
    {0x0C60, 0x4C80, 0xC600, 0x2640}  // Z
};

static const uint32_t COLORS[8] = {
    TFT_BLACK,  // 0 is empty
    TFT_CYAN,   // I
    TFT_BLUE,   // J
    TFT_ORANGE, // L
    TFT_YELLOW, // O
    TFT_GREEN,  // S
    TFT_PURPLE, // T
    TFT_RED     // Z
};

void TetrisGame::start() {
    randomSeed(esp_random());
    resetBoard();
    gameOver = false;
    score = 0;
    fallSpeed = 1000;
    heldPiece = -1;
    hasHeldThisTurn = false;
    lastFallTime = millis();
    spawnPiece();

    Serial.println("Tetris Game Started!");
    // Optional: play an initial bootup sound
    Sound::playSFX("/clash.wav");
}

void TetrisGame::resetBoard() {
    for (int r = 0; r < TETRIS_ROWS; r++) {
        for (int c = 0; c < TETRIS_COLS; c++) {
            board[r][c] = 0;
        }
    }
}

void TetrisGame::spawnPiece() {
    currentPiece = random(0, 7);
    currentRotation = 0;
    pieceX = 3; 
    pieceY = 0;

    if (checkCollision(pieceX, pieceY, currentRotation)) {
        gameOver = true;
        Sound::playSFX("/clash.wav"); // Game over sound
    }
}

bool TetrisGame::checkCollision(int px, int py, int rot) {
    uint16_t shape = SHAPES[currentPiece][rot];
    for (int i = 0; i < 16; i++) {
        if (shape & (0x8000 >> i)) {
            int gridX = px + (i % 4);
            int gridY = py + (i / 4);

            if (gridX < 0 || gridX >= TETRIS_COLS || gridY >= TETRIS_ROWS) return true; // Bounds
            if (gridY >= 0 && board[gridY][gridX] != 0) return true; // Hits another block
        }
    }
    return false;
}

void TetrisGame::lockPiece() {
    uint16_t shape = SHAPES[currentPiece][currentRotation];
    for (int i = 0; i < 16; i++) {
        if (shape & (0x8000 >> i)) {
            int gridX = pieceX + (i % 4);
            int gridY = pieceY + (i / 4);
            if (gridY >= 0) {
                board[gridY][gridX] = currentPiece + 1;
            }
        }
    }
    
    Sound::playSFX("/swing.wav"); // Lock sound
    clearLines();
    hasHeldThisTurn = false;
    spawnPiece();
}

void TetrisGame::clearLines() {
    int linesCleared = 0;
    for (int r = TETRIS_ROWS - 1; r >= 0; r--) {
        bool full = true;
        for (int c = 0; c < TETRIS_COLS; c++) {
            if (board[r][c] == 0) {
                full = false;
                break;
            }
        }

        if (full) {
            linesCleared++;
            // Shift down
            for (int r2 = r; r2 > 0; r2--) {
                for (int c = 0; c < TETRIS_COLS; c++) {
                    board[r2][c] = board[r2 - 1][c];
                }
            }
            // Clear top
            for (int c = 0; c < TETRIS_COLS; c++) {
                board[0][c] = 0;
            }
            r++; // Recheck the same line index next iteration because we shifted
        }
    }

    if (linesCleared > 0) {
        score += linesCleared * 100;
        fallSpeed = max((unsigned long)150, fallSpeed - 50); // Speed up
        Sound::playSFX("/clash.wav"); // Clear sound
    }
}

void TetrisGame::update() {
    if (gameOver) {
        if (Buttons::justPressed(BTN_START) || Buttons::justPressed(BTN_A)) {
            start(); // Restart
        }
        return;
    }

    // Input Handling
    if (Buttons::justPressed(BTN_LEFT) && !checkCollision(pieceX - 1, pieceY, currentRotation)) {
        pieceX--;
    }
    if (Buttons::justPressed(BTN_RIGHT) && !checkCollision(pieceX + 1, pieceY, currentRotation)) {
        pieceX++;
    }
    if (Buttons::justPressed(BTN_DOWN)) {
        if (!checkCollision(pieceX, pieceY + 1, currentRotation)) {
            pieceY++;
        }
        lastFallTime = millis(); // Reset fall timer
    }
    // Rotate Clockwise with A
    if (Buttons::justPressed(BTN_A)) {
        int newRot = (currentRotation + 1) % 4;
        if (!checkCollision(pieceX, pieceY, newRot)) {
            currentRotation = newRot;
            Sound::playSFX("/swing.wav");
        }
    }
    // Rotate Counter-Clockwise with B
    if (Buttons::justPressed(BTN_B)) {
        int newRot = (currentRotation + 3) % 4;
        if (!checkCollision(pieceX, pieceY, newRot)) {
            currentRotation = newRot;
            Sound::playSFX("/swing.wav");
        }
    }
    // Hard Drop with UP
    if (Buttons::justPressed(BTN_UP)) {
        while (!checkCollision(pieceX, pieceY + 1, currentRotation)) {
            pieceY++;
        }
        lockPiece();
        lastFallTime = millis();
    }
    
    // Hold piece mechanic with SELECT
    if (Buttons::justPressed(BTN_SELECT)) {
        if (!hasHeldThisTurn) {
            Sound::playSFX("/ignite.wav"); // Swap sound
            
            int temp = currentPiece;
            if (heldPiece == -1) {
                heldPiece = currentPiece;
                spawnPiece();
            } else {
                currentPiece = heldPiece;
                heldPiece = temp;
                pieceX = 3;
                pieceY = 0;
                currentRotation = 0;
                
                if (checkCollision(pieceX, pieceY, currentRotation)) {
                    gameOver = true;
                    Sound::playSFX("/clash.wav");
                }
            }
            hasHeldThisTurn = true;
        }
    }

    // Auto Fall
    if (millis() - lastFallTime > fallSpeed) {
        if (!checkCollision(pieceX, pieceY + 1, currentRotation)) {
            pieceY++;
        } else {
            lockPiece();
        }
        lastFallTime = millis();
    }
}

void TetrisGame::drawBlock(int x, int y, uint32_t color) {
    // Top-left corner for the board on a 320x240 screen:
    int startX = 110; 
    int startY = 20;

    int screenX = startX + (x * BLOCK_SIZE);
    int screenY = startY + (y * BLOCK_SIZE);

    // Draw slightly smaller to leave a grid border line
    Display::backbuffer.fillRect(screenX, screenY, BLOCK_SIZE - 1, BLOCK_SIZE - 1, color);
}

void TetrisGame::drawGhostBlock(int x, int y, uint32_t color) {
    int startX = 110; 
    int startY = 20;

    int screenX = startX + (x * BLOCK_SIZE);
    int screenY = startY + (y * BLOCK_SIZE);

    // Draw outline slightly smaller to leave a grid border line
    Display::backbuffer.drawRect(screenX, screenY, BLOCK_SIZE - 1, BLOCK_SIZE - 1, color);
}

void TetrisGame::draw() {
    Display::clear(TFT_BLACK);

    // Draw border
    int boardStartX = 110;
    int boardStartY = 20;
    Display::backbuffer.drawRect(boardStartX - 1, boardStartY - 1, (TETRIS_COLS * BLOCK_SIZE) + 2, (TETRIS_ROWS * BLOCK_SIZE) + 2, TFT_WHITE);

    // Draw locked board
    for (int r = 0; r < TETRIS_ROWS; r++) {
        for (int c = 0; c < TETRIS_COLS; c++) {
            if (board[r][c] != 0) {
                drawBlock(c, r, COLORS[board[r][c]]);
            }
        }
    }

    // Draw ghost piece (outline projection at landing spot)
    if (!gameOver) {
        int ghostY = pieceY;
        while (!checkCollision(pieceX, ghostY + 1, currentRotation)) {
            ghostY++;
        }
        
        uint16_t shape = SHAPES[currentPiece][currentRotation];
        for (int i = 0; i < 16; i++) {
            if (shape & (0x8000 >> i)) {
                int px = pieceX + (i % 4);
                int py = ghostY + (i / 4);
                if (py >= 0) drawGhostBlock(px, py, COLORS[currentPiece + 1]);
            }
        }
    }

    // Draw active piece
    if (!gameOver) {
        uint16_t shape = SHAPES[currentPiece][currentRotation];
        for (int i = 0; i < 16; i++) {
            if (shape & (0x8000 >> i)) {
                int px = pieceX + (i % 4);
                int py = pieceY + (i / 4);
                if (py >= 0) drawBlock(px, py, COLORS[currentPiece + 1]);
            }
        }
    }

    // Draw UI
    Display::backbuffer.setTextColor(TFT_WHITE, TFT_BLACK);
    Display::backbuffer.setTextSize(2);
    Display::backbuffer.setCursor(10, 20);
    Display::backbuffer.println("TETRIS");
    
    Display::backbuffer.setTextSize(1);
    Display::backbuffer.setCursor(10, 50);
    Display::backbuffer.println("Score:");
    Display::backbuffer.setCursor(10, 65);
    Display::backbuffer.println(score);

    // Draw Hold Box
    Display::backbuffer.setTextSize(1);
    Display::backbuffer.setCursor(10, 95);
    Display::backbuffer.println("HOLD:");
    
    // Draw a small border box for hold
    Display::backbuffer.drawRect(10, 110, 48, 48, TFT_DARKGREY);
    
    if (heldPiece != -1) {
        uint16_t shape = SHAPES[heldPiece][0]; // Draw default rotation
        
        // Center the 4x4 grid (32x32 pixels) inside the 48x48 box
        int startX = 18;
        int startY = 118;
        
        for (int i = 0; i < 16; i++) {
            if (shape & (0x8000 >> i)) {
                int bx = startX + (i % 4) * 8;
                int by = startY + (i / 4) * 8;
                Display::backbuffer.fillRect(bx, by, 7, 7, COLORS[heldPiece + 1]);
            }
        }
    }

    if (gameOver) {
        Display::backbuffer.setTextColor(TFT_RED, TFT_BLACK);
        Display::backbuffer.setTextSize(2);
        Display::backbuffer.setCursor(100, 100);
        Display::backbuffer.println("GAMEOVER");
        
        Display::backbuffer.setTextColor(TFT_WHITE, TFT_BLACK);
        Display::backbuffer.setTextSize(1);
        Display::backbuffer.setCursor(85, 125);
        Display::backbuffer.println("Press A to Retry");
    }
}

void TetrisGame::stop() {
    Serial.println("Tetris Game Stopped!");
}
