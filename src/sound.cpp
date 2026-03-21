#include "sound.h"

#include "config.h"

#define SOUND_QUEUE_LENGTH 4

static QueueHandle_t soundQueue;
static TaskHandle_t soundTaskHandle = nullptr;

volatile SoundType Sound::currentSoundState = SOUND_NONE;

static const char* soundFiles[] = {nullptr, "/ignite.wav", "/extinguish.wav", "/hum.wav",
                                   "/swing.wav"};

File wavFile;
wav_header_t wav_header;

void Sound::setup() {
    // Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // I2S config
    i2s_config_t i2sConfig = {.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
                              .sample_rate = 44100,
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

    // Create sound queue
    soundQueue = xQueueCreate(SOUND_QUEUE_LENGTH, sizeof(SoundType));
}

void playWav(const char* filename, volatile SoundType* soundStatePtr, SoundType thisSound) {
    if (!filename) return;

    wavFile = SPIFFS.open(filename, "r");
    if (!wavFile) {
        Serial.printf("Failed to open WAV file: %s\n", filename);
        return;
    }

    // Read WAV header
    wavFile.read((uint8_t*)&wav_header, sizeof(wav_header_t));
    if (wav_header.bit_depth != 16) {
        Serial.printf("ERROR: bit depth %d is not supported\n", wav_header.bit_depth);
        return;
    }
    i2s_set_sample_rates(I2S_NUM, wav_header.sample_rate);

    stereo_frame_t frames[FRAMES_PER_CHUNK];
    size_t bytesWritten;
    int num_channels = wav_header.num_channels;

    SoundType newCommand;
    while (wavFile.available() > 0) {
        // Check for interruption after each chunk
        if (soundStatePtr && *soundStatePtr != thisSound) {
            break;
        }
        // Check for new command in the queue (non-blocking)
        if (soundQueue && xQueueReceive(soundQueue, &newCommand, 0)) {
            // Interrupt playback if a new command is received
            *soundStatePtr = newCommand;
            break;
        }
        for (int i = 0; i < FRAMES_PER_CHUNK; i++) {
            wavFile.read((uint8_t*)&frames[i].left, sizeof(int16_t));
            if (num_channels == 1) {
                frames[i].right = frames[i].left;
            } else {
                wavFile.read((uint8_t*)&frames[i].right, sizeof(int16_t));
            }
        }
        i2s_write(I2S_NUM, (const char*)frames, sizeof(frames), &bytesWritten, portMAX_DELAY);
    }
    wavFile.close();
    if (thisSound == SOUND_SWING) {
        Sound::play(SOUND_HUM);
    }
}

void soundTask(void* pvParameters) {
    while (true) {
        SoundType requestedSound;
        // Always check for new command
        if (xQueueReceive(soundQueue, &requestedSound, 0)) {
            Sound::currentSoundState = requestedSound;
        }

        switch (Sound::currentSoundState) {
            case SOUND_IGNITE:
                playWav(soundFiles[SOUND_IGNITE], &Sound::currentSoundState, SOUND_IGNITE);
                if (Sound::currentSoundState != SOUND_IGNITE) break;
                Sound::currentSoundState = SOUND_HUM;
                break;
            case SOUND_EXTINGUISH:
                playWav(soundFiles[SOUND_EXTINGUISH], &Sound::currentSoundState, SOUND_EXTINGUISH);
                Sound::currentSoundState = SOUND_NONE;
                break;
            case SOUND_SWING:
                playWav(soundFiles[SOUND_SWING], &Sound::currentSoundState, SOUND_SWING);
                if (Sound::currentSoundState != SOUND_SWING) break;
                // Sound::currentSoundState = SOUND_HUM;
                break;
            case SOUND_HUM:
                while (Sound::currentSoundState == SOUND_HUM) {
                    playWav(soundFiles[SOUND_HUM], &Sound::currentSoundState, SOUND_HUM);
                    if (Sound::currentSoundState != SOUND_HUM) break;
                }
                break;
            case SOUND_NONE:
            default:
                vTaskDelay(10 / portTICK_PERIOD_MS);
                break;
        }
    }
}

void Sound::startTask() {
    if (!soundTaskHandle) {
        xTaskCreatePinnedToCore(soundTask, "SoundTask", 4096, NULL, 1, &soundTaskHandle, 1);
    }
}

void Sound::play(SoundType type) {
    if (soundQueue) {
        xQueueSend(soundQueue, &type, 0);
    }
}