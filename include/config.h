#pragma once

// I2S pins and settings
#define I2S_NUM I2S_NUM_0
#define I2S_BCK_IO 17
#define I2S_WS_IO 16
#define I2S_DO_IO 18

// LCD
#define LCD_DIN X
#define LCD_CLK X
#define LCD_CS X
#define LCD_DC X
#define LCD_RST X
#define LCD_BL X

// Buttons (Update GPIOs to your actual wiring later)
#define BTN_UP_PIN 15 //x
#define BTN_DOWN_PIN 41 //x
#define BTN_LEFT_PIN 42
#define BTN_RIGHT_PIN 7 //x
#define BTN_A_PIN 5
#define BTN_B_PIN 6
#define BTN_START_PIN 39
#define BTN_SELECT_PIN 40

// DMA / Audio settings
#define DMA_BUF_COUNT 8
#define DMA_BUF_LEN 512
#define FRAMES_PER_CHUNK 256
