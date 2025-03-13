#include <Arduino.h>
#include "defines.h"
#include "board.h"
#include "drivers/Drivers.h"
#include "services/Services.h"
#include "drivers/onboard/_LedAndButton.h" // temp

static bool s_buttonState = false;

////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <GyverNTP.h>

////////////////////////////////////////////////////////////////////////////////

#include "shared/image/QRCode.h"

TFT_eSprite sprite(&driver::display);

const int ver = 1;
const int scale = 5;
image::QRCode qrcode(ver);

void setup() 
{
    Serial.begin(115200);
    delay(1500);

    // // turn on the onboard led
    // #ifdef PIN_LED_CAT
    // pinMode(PIN_LED_CAT, OUTPUT);
    // digitalWrite(PIN_LED_CAT, LOW);
    // #endif

    // start hardware
    drivers::begin();
    // driver::ledAndButton.begin();

    // start services
    services::begin();

    // test
    if (driver::storage.getFSMountPoint())
    {
        Serial.printf("Strorage mount point: %s\n", driver::storage.getFSMountPoint());
        Serial.printf("Storage total bytes: %f\n", driver::storage.getFSTotalBytes() / (1024.f * 1024.f));
        Serial.printf("Storage used bytes: %f\n", driver::storage.getFSUsedBytes() / (1024.f * 1024.f));
    }

    // tft test text
    driver::display.fillScreen(TFT_NAVY);
    driver::display.setCursor(8, 8);
    driver::display.setTextColor(TFT_YELLOW);
    driver::display.setTextSize(1);
    driver::display.print("Hello World!");

    // start the audio player
    bool shuffle = true, loop = false;
    String format = "mp3", filelist = "Retrowave";
    service::audioPlayer.start(
        new service::audio_player::StorageAudioContext(format, filelist, shuffle, loop)
    );
}

void loop() 
{
    // update hardware
    // driver::ledAndButton.update();

    // update services
    service::networkConnection.update();
    service::geoLocation.update();
    service::dateAndTime.update();
    service::weatherForecast.update();

    // test
    // bool buttonState = driver::ledAndButton.getButtonState();
    // if (buttonState != s_buttonState)
    // {
    //     s_buttonState = buttonState;
    //     if (buttonState)
    //     {
    //         Serial.println("Button pressed!");
    //     }
    //     else
    //     {
    //         Serial.println("Button released!");
    //     }
    // }

    if (NTP.synced() && NTP.newSecond())
    {
        qrcode.create(NTP.timeToString().c_str());

    #if 1
        qrcode.renderOn(sprite, scale);
        int offsetX = (driver::display.width() - sprite.width()) / 2;
        int offsetY = (driver::display.height() - sprite.height()) / 2;
        sprite.pushSprite(offsetX, offsetY);
        sprite.deleteSprite();
    #else
        int gfxSize = qrcode.getGfxSize(scale);
        int offsetX = (driver::display.width() - gfxSize) / 2;
        int offsetY = (driver::display.height() - gfxSize) / 2;
        qrcode.renderOn(driver::display, scale, offsetX, offsetY);
    #endif
    }
}
