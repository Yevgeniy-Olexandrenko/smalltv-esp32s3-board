#include <Arduino.h>

// hardware drivers
#include "drivers/PowerSource.h"
#include "drivers/LedAndButton.h"
#include "drivers/storage/SDCard.h"
#include "drivers/storage/SDCardMSC.h"
#include "drivers/storage/Flash.h"

// background services
#include "services/NetworkConnection.h"
#include "services/GeoLocation.h"
#include "services/DateAndTime.h"
#include "services/Weather.h"

// webserver app
#include "webserver/SettingsWebApp.h"

// test
#include <LittleFS.h>

#define ONBOARD_LED GPIO_NUM_0
#define DISPLAY_BACKLIGHT GPIO_NUM_14

// mmc
#define SDCARD_MMC_DET GPIO_NUM_47
#define SDCARD_MMC_CLK GPIO_NUM_41
#define SDCARD_MMC_CMD GPIO_NUM_38
#define SDCARD_MMC_D0  GPIO_NUM_42
#define SDCARD_MMC_D1  GPIO_NUM_21
#define SDCARD_MMC_D2  GPIO_NUM_40
#define SDCARD_MMC_D3  GPIO_NUM_39

// spi
#define SDCARD_SPI_MISO SDCARD_MMC_D0
#define SDCARD_SPI_MOSI SDCARD_MMC_CMD
#define SDCARD_SPI_CLK  SDCARD_MMC_CLK
#define SDCARD_SPI_CS   SDCARD_MMC_D3

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

    // driver::storage.begin();
    // Serial.print("SD Card availavle: ");
    // Serial.println(driver::storage.isSDCardAvailable() ? "YES" : "NO");

    driver::sdcard.begin
    (
        driver::SDCard::DEFAULT_MOUNT_POINT,
        SDCARD_MMC_CLK,
        SDCARD_MMC_CMD,
        SDCARD_MMC_D0,
        SDCARD_MMC_D1,
        SDCARD_MMC_D2,
        SDCARD_MMC_D3
    );
    // driver::sdcard.begin
    // (
    //     driver::SDCard::DEFAULT_MOUNT_POINT,
    //     SDCARD_MMC_CLK,
    //     SDCARD_MMC_CMD,
    //     SDCARD_MMC_D0
    // );
    // driver::sdcard.begin
    // (
    //     driver::SDCard::DEFAULT_MOUNT_POINT,
    //     SDCARD_SPI_MISO,
    //     SDCARD_SPI_MOSI,
    //     SDCARD_SPI_CLK,
    //     SDCARD_SPI_CS
    // );
    Serial.print("SD Card size: ");
    // Serial.println((driver::sdcard.getSectorSize() * driver::sdcard.getSectorCount()) / (1024.f * 1024.f));
    Serial.println(driver::sdcard.getPartitionSize() / (1024.f * 1024.f));
    Serial.print("SD Card sector size: ");
    Serial.println(driver::sdcard.getSectorSize());
    Serial.print("SD Card sector count: ");
    Serial.println(driver::sdcard.getSectorCount());

    // test
    LittleFS.begin(false);
    Serial.print("LittleFS size: ");
    Serial.println(LittleFS.totalBytes() / (1024.f * 1024.f));

    driver::flash.begin(driver::Flash::DEFAULT_MOUNT_POINT);
    Serial.print("Flash FAT size: ");
    Serial.println(driver::flash.getTotalBytes() / (1024.f * 1024.f));

    // start services
    service::networkConnection.begin();
    service::GeoLocation.begin();
    service::DateAndTime.begin();
    service::Weather.begin();

    // start webserver app
    webserver::SettingsWebApp.begin();

    // test
    // list_dir("/sdcard", 0);
    // list_dir("/littlefs", 0);
    list_dir("/flash", 0);
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
            if (driver::sdcardmsc.isRunning())
                driver::sdcardmsc.stopMSC();
            else
                driver::sdcardmsc.startMSC(true);
        }
        else
        {
            Serial.println("Button released!");
        }
    }
}
