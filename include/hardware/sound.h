#pragma once

#include <Arduino.h>
#include <SPIFFS.h>
#include <driver/i2s.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

// WAV header struct
typedef struct {
    char riff_header[4];
    int wav_size;
    char wave_header[4];
    char fmt_header[4];
    int fmt_chunk_size;
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short sample_alignment;
    short bit_depth;
    char data_header[4];
    int data_bytes;
} wav_header_t;

// Stereo frame struct
typedef struct {
    int16_t left;
    int16_t right;
} stereo_frame_t;

enum SoundCommandType {
    CMD_PLAY_BGM,
    CMD_STOP_BGM,
    CMD_PLAY_SFX
};

struct SoundCommand {
    SoundCommandType type;
    char filename[32];
    bool loop; // Only applies to BGM
};

namespace Sound {
    void setup();
    void startTask();
    
    // Play background music (replaces current BGM)
    void playBGM(const char* filename, bool loop = true);
    void stopBGM();
    
    // Play a sound effect over the background music
    void playSFX(const char* filename);

    // Get and set sound volume (0-100)
    int getVolume();
    void setVolume(int volume);
}