#include "hardware/sound.h"
#include "core/config.h"

#define SOUND_QUEUE_LENGTH 10

static QueueHandle_t soundQueue;
static TaskHandle_t soundTaskHandle = nullptr;

// We will keep two file objects for the software mixer
File bgmFile;
File sfxFile;

wav_header_t bgm_header;
wav_header_t sfx_header;

bool isBgmPlaying = false;
bool isSfxPlaying = false;
bool loopBgm = false;
char currentBgmFilename[32] = {0};

void Sound::setup() {
    // Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed in Sound::setup()");
        return;
    }

    // I2S config
    i2s_config_t i2sConfig = {.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
                              .sample_rate = 44100, // Standard WAV rate
                              .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
                              .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
                              .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S,
                              .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
                              .dma_buf_count = DMA_BUF_COUNT,
                              .dma_buf_len = DMA_BUF_LEN,
                              .use_apll = false,
                              .tx_desc_auto_clear = true,
                              .fixed_mclk = 0};

    // I2S pin config
    i2s_pin_config_t pinConfig = {.bck_io_num = I2S_BCK_IO,
                                  .ws_io_num = I2S_WS_IO,
                                  .data_out_num = I2S_DO_IO,
                                  .data_in_num = I2S_PIN_NO_CHANGE};

    i2s_driver_install(I2S_NUM, &i2sConfig, 0, NULL);
    i2s_set_pin(I2S_NUM, &pinConfig);

    // Create command queue
    soundQueue = xQueueCreate(SOUND_QUEUE_LENGTH, sizeof(SoundCommand));
}

// Simple clipping helper function
int16_t clipAudio(int32_t sample) {
    if (sample > 32767) return 32767;
    if (sample < -32768) return -32768;
    return (int16_t)sample;
}

void soundTask(void* pvParameters) {
    stereo_frame_t mixFrames[FRAMES_PER_CHUNK];
    size_t bytesWritten;
    
    while (true) {
        SoundCommand cmd;
        
        if (xQueueReceive(soundQueue, &cmd, 0)) {
            if (cmd.type == CMD_PLAY_BGM) {
                if (bgmFile) bgmFile.close();
                strncpy(currentBgmFilename, cmd.filename, 31);
                bgmFile = SPIFFS.open(cmd.filename, "r");
                if (bgmFile) {
                    size_t bytesRead = bgmFile.read((uint8_t*)&bgm_header, sizeof(wav_header_t));
                    if (bytesRead == sizeof(wav_header_t) && bgm_header.sample_rate > 0) {
                        isBgmPlaying = true;
                        loopBgm = cmd.loop;
                        i2s_set_sample_rates(I2S_NUM, bgm_header.sample_rate);
                    } else {
                        Serial.printf("Invalid or empty BGM WAV file: %s\n", cmd.filename);
                        bgmFile.close();
                        isBgmPlaying = false;
                    }
                } else {
                    Serial.printf("Failed to open BGM: %s\n", cmd.filename);
                }
            } else if (cmd.type == CMD_STOP_BGM) {
                if (bgmFile) bgmFile.close();
                isBgmPlaying = false;
            } else if (cmd.type == CMD_PLAY_SFX) {
                if (sfxFile) sfxFile.close();
                sfxFile = SPIFFS.open(cmd.filename, "r");
                if (sfxFile) {
                    size_t bytesRead = sfxFile.read((uint8_t*)&sfx_header, sizeof(wav_header_t));
                    if (bytesRead == sizeof(wav_header_t) && sfx_header.sample_rate > 0) {
                        isSfxPlaying = true;
                    } else {
                        Serial.printf("Invalid or empty SFX WAV file: %s\n", cmd.filename);
                        sfxFile.close();
                        isSfxPlaying = false;
                    }
                } else {
                    Serial.printf("Failed to open SFX: %s\n", cmd.filename);
                }
            }
        }

        memset(mixFrames, 0, sizeof(mixFrames));
        bool hasAudio = false;

        // Process BGM chunk
        if (isBgmPlaying && bgmFile) {
            int bytesPerFrame = (bgm_header.num_channels == 1) ? 2 : 4;
            int toRead = FRAMES_PER_CHUNK * bytesPerFrame;
            uint8_t readBuf[FRAMES_PER_CHUNK * 4];
            
            int bytesRead = bgmFile.read(readBuf, toRead);
            
            if (bytesRead < bytesPerFrame) {
                if (loopBgm) {
                    bgmFile.seek(sizeof(wav_header_t));
                } else {
                    isBgmPlaying = false;
                    bgmFile.close();
                }
            } else {
                int framesRead = bytesRead / bytesPerFrame;
                int bufIdx = 0;
                for (int i = 0; i < framesRead; i++) {
                    int16_t sampleLeft = *((int16_t*)(readBuf + bufIdx));
                    bufIdx += 2;
                    int16_t sampleRight = sampleLeft;
                    if (bgm_header.num_channels == 2) {
                        sampleRight = *((int16_t*)(readBuf + bufIdx));
                        bufIdx += 2;
                    }
                    mixFrames[i].left = sampleLeft;
                    mixFrames[i].right = sampleRight;
                }
                hasAudio = true;
            }
        }

        // Process SFX chunk and Mix
        if (isSfxPlaying && sfxFile) {
            int bytesPerFrame = (sfx_header.num_channels == 1) ? 2 : 4;
            int toRead = FRAMES_PER_CHUNK * bytesPerFrame;
            uint8_t readBuf[FRAMES_PER_CHUNK * 4];
            
            int bytesRead = sfxFile.read(readBuf, toRead);
            
            if (bytesRead < bytesPerFrame) {
                isSfxPlaying = false;
                sfxFile.close();
            } else {
                int framesRead = bytesRead / bytesPerFrame;
                int bufIdx = 0;
                for (int i = 0; i < framesRead; i++) {
                    int16_t sampleLeft = *((int16_t*)(readBuf + bufIdx));
                    bufIdx += 2;
                    int16_t sampleRight = sampleLeft;
                    if (sfx_header.num_channels == 2) {
                        sampleRight = *((int16_t*)(readBuf + bufIdx));
                        bufIdx += 2;
                    }
                    // Mix samples directly by adding and clipping
                    mixFrames[i].left = clipAudio((int32_t)mixFrames[i].left + (int32_t)sampleLeft);
                    mixFrames[i].right = clipAudio((int32_t)mixFrames[i].right + (int32_t)sampleRight);
                }
                hasAudio = true;
            }
        }

        if (hasAudio) {
            i2s_write(I2S_NUM, (const char*)mixFrames, sizeof(mixFrames), &bytesWritten, portMAX_DELAY);
        } else {
            // Nothing to play, rest for bit
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void Sound::startTask() {
    if (!soundTaskHandle) {
        // Run on Core 0 (background) instead of Core 1 (game loop) to prevent game lagging
        xTaskCreatePinnedToCore(soundTask, "SoundTask", 16384, NULL, 2, &soundTaskHandle, 0);
    }
}

void Sound::playBGM(const char* filename, bool loop) {
    if (soundQueue) {
        SoundCommand cmd;
        cmd.type = CMD_PLAY_BGM;
        cmd.loop = loop;
        strncpy(cmd.filename, filename, 31);
        xQueueSend(soundQueue, &cmd, 0);
    }
}

void Sound::stopBGM() {
    if (soundQueue) {
        SoundCommand cmd;
        cmd.type = CMD_STOP_BGM;
        xQueueSend(soundQueue, &cmd, 0);
    }
}

void Sound::playSFX(const char* filename) {
    if (soundQueue) {
        SoundCommand cmd;
        cmd.type = CMD_PLAY_SFX;
        strncpy(cmd.filename, filename, 31);
        xQueueSend(soundQueue, &cmd, 0);
    }
}