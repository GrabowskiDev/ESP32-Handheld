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

enum SoundType { SOUND_NONE, SOUND_IGNITE, SOUND_EXTINGUISH, SOUND_HUM, SOUND_SWING };

namespace Sound {
void setup();
void play(SoundType type);
void startTask();
// For immediate sound interruption
extern volatile SoundType currentSoundState;
}  // namespace Sound