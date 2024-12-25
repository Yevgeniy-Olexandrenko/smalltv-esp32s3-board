#include <Arduino.h>

// hardware drivers
#include "drivers/PowerSource.h"
#include "drivers/LedAndButton.h"
#include "drivers/storage/SDCard.h"
#include "drivers/storage/SDCardMSC.h"

// background services
#include "services/NetworkConnection.h"
#include "services/GeoLocation.h"
#include "services/DateAndTime.h"
#include "services/Weather.h"

// webserver app
#include "webserver/SettingsWebApp.h"

// test
#include <LittleFS.h>
#include <FFat.h>

#define ONBOARD_LED GPIO_NUM_0
#define DISPLAY_BACKLIGHT GPIO_NUM_14

#define SDCARD_MMC_DET GPIO_NUM_47
#define SDCARD_MMC_CLK GPIO_NUM_41
#define SDCARD_MMC_CMD GPIO_NUM_38
#define SDCARD_MMC_D0  GPIO_NUM_42
#define SDCARD_MMC_D1  GPIO_NUM_21
#define SDCARD_MMC_D2  GPIO_NUM_40
#define SDCARD_MMC_D3  GPIO_NUM_39


static bool s_buttonState = false;

void listDir(fs::FS& fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.path(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
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
        SDCARD_MMC_CLK,
        SDCARD_MMC_CMD,
        SDCARD_MMC_D0,
        SDCARD_MMC_D1,
        SDCARD_MMC_D2,
        SDCARD_MMC_D3
    );
    // driver::sdcard.begin
    // (
    //     SDCARD_MMC_CLK,
    //     SDCARD_MMC_CMD,
    //     SDCARD_MMC_D0
    // );
    Serial.print("SD Card size: ");
    Serial.println((driver::sdcard.getSectorSize() * driver::sdcard.getSectorCount()) / (1024.f * 1024.f));
    Serial.print("SD Card sector size: ");
    Serial.println(driver::sdcard.getSectorSize());
    Serial.print("SD Card sector count: ");
    Serial.println(driver::sdcard.getSectorCount());

    // test
    LittleFS.begin(false);
    Serial.print("LittleFS size: ");
    Serial.println(LittleFS.totalBytes() / (1024.f * 1024.f));

    FFat.begin(true);
    Serial.print("FFat size: ");
    Serial.println(FFat.totalBytes() / (1024.f * 1024.f));

    // start services
    service::networkConnection.begin();
    service::GeoLocation.begin();
    service::DateAndTime.begin();
    service::Weather.begin();

    // start webserver app
    webserver::SettingsWebApp.begin();

    // test
//    listDir(driver::sdcard, "/", 10);
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
                driver::sdcardmsc.end();
            else
                driver::sdcardmsc.begin();
        }
        else
        {
            Serial.println("Button released!");
        }
    }
}
