#include "games/MainMenu.h"
#include "hardware/display.h"
#include "hardware/buttons.h"
#include "hardware/sound.h"
#include "games/TetrisGame.h"
#include "games/ExampleGame.h"
#include "core/GameManager.h"
#include <SPIFFS.h>

bool MainMenu::bgmEnabled = true;

void MainMenu::start() {
    activeScreen = SCREEN_MAIN;
    selectedItem = 0;
    settingsCursor = 0;
    bgOffset = 0;
    startMillis = millis();
    
    // Play background menu music
    if (bgmEnabled) {
        Sound::playBGM("/hum.wav", true);
    } else {
        Sound::stopBGM();
    }
    
    // Read low-level ESP32 specs
    refreshSystemStats();
    
    Serial.println("MainMenu: Started!");
}

void MainMenu::refreshSystemStats() {
    freeHeap = ESP.getFreeHeap();
    totalHeap = ESP.getHeapSize();
    cpuSpeedMhz = ESP.getCpuFreqMHz();
    chipModel = String(ESP.getChipModel());
}

void MainMenu::update() {
    bgOffset++; // Animate background grid
    
    if (activeScreen == SCREEN_MAIN) {
        updateMainMenu();
    } else {
        updateSettingsMenu();
    }
}

void MainMenu::updateMainMenu() {
    // Grace period to prevent button carryovers from exited games
    if (millis() - startMillis < 350) {
        return;
    }

    if (Buttons::justPressed(BTN_DOWN)) {
        selectedItem = (selectedItem + 1) % 3;
        Sound::playSFX("/swing.wav");
    }
    if (Buttons::justPressed(BTN_UP)) {
        selectedItem = (selectedItem + 2) % 3; // equivalent to -1
        Sound::playSFX("/swing.wav");
    }
    
    if (Buttons::justPressed(BTN_A) || Buttons::justPressed(BTN_START)) {
        Sound::playSFX("/ignite.wav");
        delay(150); // Let sound start playing before switching context
        
        if (selectedItem == 0) {
            gameManager.switchGame(new TetrisGame());
        } else if (selectedItem == 1) {
            gameManager.switchGame(new ExampleGame());
        } else if (selectedItem == 2) {
            activeScreen = SCREEN_SETTINGS;
            settingsCursor = 0;
            refreshSystemStats();
            startMillis = millis(); // Reset grace period for settings screen
        }
    }
}

void MainMenu::updateSettingsMenu() {
    // Grace period check
    if (millis() - startMillis < 350) {
        return;
    }

    if (Buttons::justPressed(BTN_DOWN)) {
        settingsCursor = (settingsCursor + 1) % 3;
        Sound::playSFX("/swing.wav");
    }
    if (Buttons::justPressed(BTN_UP)) {
        settingsCursor = (settingsCursor + 2) % 3; // equivalent to -1
        Sound::playSFX("/swing.wav");
    }
    
    // Adjust brightness on item 0 using D-pad LEFT/RIGHT
    if (settingsCursor == 0) {
        if (Buttons::justPressed(BTN_LEFT)) {
            int p = Display::getBrightness() - 5;
            if (p < 10) p = 10; // Clamped minimum so screen is still readable
            Display::setBrightness(p);
            Sound::playSFX("/swing.wav");
        }
        if (Buttons::justPressed(BTN_RIGHT)) {
            int p = Display::getBrightness() + 5;
            if (p > 100) p = 100;
            Display::setBrightness(p);
            Sound::playSFX("/swing.wav");
        }
    }
    
    if (Buttons::justPressed(BTN_A) || Buttons::justPressed(BTN_START)) {
        Sound::playSFX("/ignite.wav");
        if (settingsCursor == 1) {
            bgmEnabled = !bgmEnabled;
            if (bgmEnabled) {
                Sound::playBGM("/hum.wav", true);
            } else {
                Sound::stopBGM();
            }
        } else if (settingsCursor == 2) {
            activeScreen = SCREEN_MAIN;
            selectedItem = 2; // Keep highlight on "Settings"
            startMillis = millis(); // Reset grace period returning to main menu
        }
    }
    
    if (Buttons::justPressed(BTN_B) || Buttons::justPressed(BTN_SELECT)) {
        Sound::playSFX("/extinguish.wav");
        activeScreen = SCREEN_MAIN;
        selectedItem = 2;
        startMillis = millis(); // Reset grace period returning to main menu
    }
}

void MainMenu::draw() {
    // 1. Draw animated backdrop
    drawBackgroundGrid();
    
    // 2. Draw active menu view
    if (activeScreen == SCREEN_MAIN) {
        drawMainMenu();
    } else {
        drawSettingsMenu();
    }
}

void MainMenu::drawBackgroundGrid() {
    Display::clear(TFT_BLACK);
    
    // Horizon division
    int horizon = 85;
    
    // Draw neon horizon glow
    Display::backbuffer.drawFastHLine(0, horizon, 320, TFT_NAVY);
    Display::backbuffer.drawFastHLine(0, horizon + 1, 320, TFT_PURPLE);
    
    // Render space starfield (top screen portion)
    // Deterministic pseudo-random stars using modular arithmetic on constants
    for (int i = 0; i < 35; i++) {
        int sx = (i * 127 + 23) % 320;
        int sy = (i * 59 + 7) % horizon;
        // Subtle twinkling star animation based on bgOffset
        uint16_t starColor = ((bgOffset + i * 5) % 40 < 20) ? TFT_DARKGREY : TFT_WHITE;
        Display::backbuffer.drawPixel(sx, sy, starColor);
    }
    
    // Render perspective synthwave grid below horizon
    // Horizontal lines scrolling down (perspective simulation)
    int lineSpacing = 16;
    for (int y = horizon + 2; y < 240; y += lineSpacing) {
        int scrollY = y + (bgOffset % lineSpacing);
        if (scrollY < 240) {
            // Deeper color at the back (gradient fading effect)
            uint16_t color = (scrollY < horizon + 30) ? TFT_NAVY : TFT_MAGENTA;
            Display::backbuffer.drawFastHLine(0, scrollY, 320, color);
        }
    }
    
    // Vertical perspective grid lines converging towards center horizon
    int centerX = 160;
    for (int offset = -200; offset <= 200; offset += 40) {
        int xBottom = centerX + offset;
        // Linear interpolation to make lines converge at the center horizon
        int xTop = centerX + (offset / 5); 
        Display::backbuffer.drawLine(xTop, horizon + 2, xBottom, 240, TFT_PURPLE);
    }
}

void MainMenu::drawMainMenu() {
    // 1. Neon Glow Header Title
    int titleX = 40;
    int titleY = 20;
    
    // Retro drop shadow effect
    Display::backbuffer.setTextColor(TFT_MAGENTA, TFT_BLACK);
    Display::backbuffer.setTextSize(3);
    Display::backbuffer.setCursor(titleX + 2, titleY + 2);
    Display::backbuffer.print("ESP32 PLAY");
    
    Display::backbuffer.setTextColor(TFT_CYAN, TFT_BLACK);
    Display::backbuffer.setCursor(titleX, titleY);
    Display::backbuffer.print("ESP32 PLAY");
    
    Display::backbuffer.setTextSize(1);
    Display::backbuffer.setTextColor(TFT_YELLOW, TFT_BLACK);
    Display::backbuffer.setCursor(80, 52);
    Display::backbuffer.print("HANDHELD CONSOLE OS");
    
    // 2. Draw Menu Items
    const char* items[3] = {
        "  TETRIS GAME  ",
        "  DEMO SCENE   ",
        "  SYSTEM SETTINGS"
    };
    
    const uint16_t itemColors[3] = { TFT_CYAN, TFT_YELLOW, TFT_MAGENTA };
    
    int startY = 110;
    int spacingY = 40;
    
    for (int i = 0; i < 3; i++) {
        int itemY = startY + (i * spacingY);
        bool isSelected = (selectedItem == i);
        
        // Render item card background
        if (isSelected) {
            // Pulse selection box
            uint16_t glowColor = ((bgOffset / 8) % 2 == 0) ? TFT_WHITE : itemColors[i];
            
            Display::backbuffer.fillRoundRect(28, itemY - 6, 264, 30, 4, TFT_NAVY);
            Display::backbuffer.drawRoundRect(28, itemY - 6, 264, 30, 4, glowColor);
            
            // Neon cursor arrow
            Display::backbuffer.setTextColor(TFT_WHITE, TFT_NAVY);
            Display::backbuffer.setTextSize(2);
            Display::backbuffer.setCursor(35, itemY + 1);
            Display::backbuffer.print(">");
            
            Display::backbuffer.setCursor(55, itemY + 1);
        } else {
            Display::backbuffer.fillRoundRect(30, itemY - 5, 260, 28, 4, Display::backbuffer.color565(20, 20, 30));
            Display::backbuffer.drawRoundRect(30, itemY - 5, 260, 28, 4, TFT_DARKGREY);
            
            Display::backbuffer.setTextColor(TFT_LIGHTGREY, Display::backbuffer.color565(20, 20, 30));
            Display::backbuffer.setTextSize(2);
            Display::backbuffer.setCursor(55, itemY + 1);
        }
        
        // Print option label
        Display::backbuffer.print(items[i]);
    }
}

void MainMenu::drawSettingsMenu() {
    // 1. Settings Header
    Display::backbuffer.setTextColor(TFT_YELLOW, TFT_BLACK);
    Display::backbuffer.setTextSize(2);
    Display::backbuffer.setCursor(20, 15);
    Display::backbuffer.print("SYSTEM SETTINGS");
    
    Display::backbuffer.drawFastHLine(15, 38, 290, TFT_YELLOW);
    
    // 2. Settings Items
    int startY = 48;
    
    // Setting 1: Display Brightness
    bool isBrightSelected = (settingsCursor == 0);
    if (isBrightSelected) {
        Display::backbuffer.fillRoundRect(15, startY - 4, 290, 24, 3, TFT_NAVY);
        Display::backbuffer.drawRoundRect(15, startY - 4, 290, 24, 3, TFT_CYAN);
        Display::backbuffer.setTextColor(TFT_WHITE, TFT_NAVY);
    } else {
        Display::backbuffer.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    }
    Display::backbuffer.setTextSize(1.5);
    Display::backbuffer.setCursor(25, startY + 4);
    Display::backbuffer.print("Brightness:");
    
    // Draw graphical slider for brightness
    int percent = Display::getBrightness();
    
    // Graphical slider coordinates
    int sliderX = 130;
    int sliderY = startY + 8;
    int sliderW = 100;
    int sliderH = 4;
    
    // Slider track
    Display::backbuffer.fillRect(sliderX, sliderY, sliderW, sliderH, TFT_DARKGREY);
    // Filled progress
    int progressW = (percent * sliderW) / 100;
    Display::backbuffer.fillRect(sliderX, sliderY, progressW, sliderH, isBrightSelected ? TFT_CYAN : TFT_LIGHTGREY);
    // Slider handle
    Display::backbuffer.fillRect(sliderX + progressW - 3, sliderY - 3, 6, 10, TFT_WHITE);
    
    // Percentage text
    Display::backbuffer.setCursor(245, startY + 4);
    Display::backbuffer.printf("%3d%%", percent);

    // Setting 2: BGM Mute
    int bgmY = startY + 28;
    bool isBgmSelected = (settingsCursor == 1);
    if (isBgmSelected) {
        Display::backbuffer.fillRoundRect(15, bgmY - 4, 290, 24, 3, TFT_NAVY);
        Display::backbuffer.drawRoundRect(15, bgmY - 4, 290, 24, 3, TFT_CYAN);
        Display::backbuffer.setTextColor(TFT_WHITE, TFT_NAVY);
    } else {
        Display::backbuffer.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    }
    Display::backbuffer.setCursor(25, bgmY + 4);
    Display::backbuffer.printf("Background Music: %s", bgmEnabled ? "PLAYING [ON]" : "MUTED  [OFF]");
    
    // Setting 3: Back to Main Menu
    int backY = bgmY + 28;
    bool isBackSelected = (settingsCursor == 2);
    if (isBackSelected) {
        Display::backbuffer.fillRoundRect(15, backY - 4, 290, 24, 3, TFT_NAVY);
        Display::backbuffer.drawRoundRect(15, backY - 4, 290, 24, 3, TFT_CYAN);
        Display::backbuffer.setTextColor(TFT_WHITE, TFT_NAVY);
    } else {
        Display::backbuffer.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    }
    Display::backbuffer.setCursor(25, backY + 4);
    Display::backbuffer.print("<< Save & Back to Menu");
    
    // 3. LOW LEVEL HARDWARE MONITOR PANEL (WOW factor)
    int hwY = 138;
    
    // Border
    Display::backbuffer.fillRoundRect(15, hwY, 290, 96, 5, Display::backbuffer.color565(10, 15, 25));
    Display::backbuffer.drawRoundRect(15, hwY, 290, 96, 5, TFT_MAGENTA);
    
    // Header
    Display::backbuffer.setTextColor(TFT_MAGENTA, Display::backbuffer.color565(10, 15, 25));
    Display::backbuffer.setTextSize(1);
    Display::backbuffer.setCursor(25, hwY + 6);
    Display::backbuffer.print("CHIP HARDWARE DASHBOARD");
    Display::backbuffer.drawFastHLine(20, hwY + 16, 280, TFT_DARKGREY);
    
    // Specs
    Display::backbuffer.setTextColor(TFT_GREEN, Display::backbuffer.color565(10, 15, 25));
    Display::backbuffer.setTextSize(1);
    
    Display::backbuffer.setCursor(25, hwY + 22);
    Display::backbuffer.printf("Chip Model: %s", chipModel.c_str());
    
    Display::backbuffer.setCursor(25, hwY + 34);
    Display::backbuffer.printf("CPU Clock:  %u MHz", cpuSpeedMhz);
    
    // Memory conversion KB
    float freeHeapKb = freeHeap / 1024.0;
    float totalHeapKb = totalHeap / 1024.0;
    Display::backbuffer.setCursor(25, hwY + 46);
    Display::backbuffer.printf("RAM Heap:   %.1f KB / %.1f KB", freeHeapKb, totalHeapKb);
    
    // SPIFFS Disk Info
    size_t totalS = SPIFFS.totalBytes();
    size_t usedS = SPIFFS.usedBytes();
    float totalKb = totalS / 1024.0;
    float usedKb = usedS / 1024.0;
    
    Display::backbuffer.setCursor(25, hwY + 58);
    Display::backbuffer.printf("Disk Size:  %.1f KB", totalKb);
    
    Display::backbuffer.setCursor(25, hwY + 70);
    Display::backbuffer.printf("Disk Used:  %.1f KB (%.1f%%)", usedKb, (float)usedS / (float)totalS * 100.0);
    
    // OS Core
    Display::backbuffer.setCursor(25, hwY + 82);
    Display::backbuffer.print("OS Version: Gemini Handheld v1.0.0");
}

void MainMenu::stop() {
    Serial.println("MainMenu: Stopped/Cleaned up!");
}
