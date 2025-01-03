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
}
