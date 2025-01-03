#include <Arduino.h>

// hardware drivers
#include "drivers/PowerSource.h"
#include "drivers/LedAndButton.h"
#include "drivers/storage/Storage.h"

// background services
#include "services/NetworkConnection.h"
#include "services/GeoLocation.h"
#include "services/DateAndTime.h"
#include "services/Weather.h"

// webserver app
#include "webserver/SettingsWebApp.h"
#include "webserver/StorageSettings.h"

// test
#include <LittleFS.h>

#define ONBOARD_LED GPIO_NUM_0
#define DISPLAY_BACKLIGHT GPIO_NUM_14

// spi
// #define SDCARD_SPI_MISO SDCARD_MMC_D0
// #define SDCARD_SPI_MOSI SDCARD_MMC_CMD
// #define SDCARD_SPI_CLK  SDCARD_MMC_CLK
// #define SDCARD_SPI_CS   SDCARD_MMC_D3

static bool s_buttonState = false;

////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

void list_dir(const char *path, int level) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        printf("Could not open dir: %s\n", path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем "." и ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Формируем полный путь к файлу/каталогу
        char full_path[256];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Получаем информацию о файле/каталоге
        struct stat entry_info;
        if (stat(full_path, &entry_info) == 0) {
            // Выводим отступы для подкаталогов
            for (int i = 0; i < level; i++) {
                printf("  ");
            }

            if (S_ISDIR(entry_info.st_mode)) {
                printf("[DIR]  %s\n", entry->d_name);
                // Рекурсивно обходим подкаталог
                list_dir(full_path, level + 1);
            } else {
                printf("[FILE] %s\n", entry->d_name);
            }
        } else {
            printf("Could not get file info: %s\n", full_path);
        }
    }
    closedir(dir);
}

////////////////////////////////////////////////////////////////////////////////

#include <driver/i2s.h>
#include <math.h>

#define I2S_PORT        I2S_NUM_0 // Используемый I2S порт
#define I2S_BCK_PIN     16
#define I2S_WS_PIN      17
#define I2S_DO_PIN      15
#define SAMPLE_RATE     44100     // Частота дискретизации
#define TONE_FREQUENCY  2000      // Частота генерируемого тона в Гц

void sound_setup() {
    // Конфигурация I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S, // Обновленное значение
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    // Конфигурация пинов I2S
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DO_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    // Установка драйвера I2S
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_set_clk(I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void sound_loop() {
    const int amplitude = 10000; // Амплитуда сигнала
    const int samples_per_cycle = SAMPLE_RATE / TONE_FREQUENCY;
    int16_t sample;
    size_t bytes_written;

    for (int i = 0; i < samples_per_cycle; ++i) {
        sample = (int16_t)(amplitude * sinf((2.0f * M_PI * i) / samples_per_cycle));
        i2s_write(I2S_PORT, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    }
}

////////////////////////////////////////////////////////////////////////////////

void setup() 
{
    Serial.begin(115200);
    delay(1500);

    // turn on the onboard led
    pinMode(ONBOARD_LED, OUTPUT);
    digitalWrite(ONBOARD_LED, LOW);

    // turn on the display backlight
    pinMode(DISPLAY_BACKLIGHT, OUTPUT);
    digitalWrite(DISPLAY_BACKLIGHT, HIGH);

    // start hardware
    driver::powerSource.begin();
    driver::ledAndButton.begin();
    driver::storage.begin(webserver::StorageSettings.getStorageType());

    // start services
    service::networkConnection.begin();
    service::GeoLocation.begin();
    service::DateAndTime.begin();
    service::Weather.begin();

    // start webserver app
    webserver::SettingsWebApp.begin();

    // test
    // list_dir(driver::storage.getFSMountPoint(), 0);
    if (driver::storage.getFSMountPoint())
    {
        Serial.printf("Strorage mount point: %s\n", driver::storage.getFSMountPoint());
        Serial.printf("Storage total bytes: %f\n", driver::storage.getFSTotalBytes() / (1024.f * 1024.f));
        Serial.printf("Storage used bytes: %f\n", driver::storage.getFSUsedBytes() / (1024.f * 1024.f));
    }

    // test sound
    sound_setup();
}

void loop() 
{
    // update hardware
    driver::ledAndButton.update();

    // update services
    service::networkConnection.update();
    service::GeoLocation.update();
    service::DateAndTime.update();
    service::Weather.update();

    // update webserver app
    webserver::SettingsWebApp.update();

    // test
    bool buttonState = driver::ledAndButton.getButtonState();
    if (buttonState != s_buttonState)
    {
        s_buttonState = buttonState;
        if (buttonState)
        {
            Serial.println("Button pressed!");
            if (!driver::storage.isMSCRunning())
            {
                driver::storage.startMSC();
            }
        }
        else
        {
            Serial.println("Button released!");
        }
    }

    // test sound
    sound_loop();
}
