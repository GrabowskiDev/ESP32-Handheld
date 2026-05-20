#pragma once

#include "../core/IGame.h"
#include <Arduino.h>

enum MenuScreen {
    SCREEN_MAIN,
    SCREEN_SETTINGS
};

class MainMenu : public IGame {
private:
    MenuScreen activeScreen;
    int selectedItem;
    int settingsCursor;
    static bool bgmEnabled;
    uint32_t bgOffset;
    unsigned long startMillis;
    
    // Low level stats
    uint32_t freeHeap;
    uint32_t totalHeap;
    uint32_t cpuSpeedMhz;
    String chipModel;
    
    void updateMainMenu();
    void updateSettingsMenu();
    
    void drawMainMenu();
    void drawSettingsMenu();
    void drawBackgroundGrid();
    
    void refreshSystemStats();

public:
    bool isMainMenu() const override { return true; }
    void start() override;
    void update() override;
    void draw() override;
    void stop() override;
};
