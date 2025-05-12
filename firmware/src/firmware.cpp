#include <Arduino.h>
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

#ifndef NO_VIDEO
#if 1
#include "shared/image/Spectrum.h"
TFT_eSprite sprite(&driver::display);
image::Spectrum spectrum(30, 80, 16000);
#else
#include "shared/image/QRCode.h"
TFT_eSprite sprite(&driver::display);
const int ver = 1;
const int scale = 5;
image::QRCode qrcode(ver);
#endif
unsigned long lastDraw = 0;
#endif

void setup() 
{
    Serial.begin(115200);
    delay(1500);

    // turn on the onboard led
    // #ifdef PIN_LED_CAT
    // pinMode(PIN_LED_CAT, OUTPUT);
    // digitalWrite(PIN_LED_CAT, LOW);
    // #endif

    drivers::begin();
    services::begin();

    // test
    if (driver::storage.getFS().isMounted())
    {
        Serial.printf("Strorage mount point: %s\n", driver::storage.getFS().mountPoint());
        Serial.printf("Storage total bytes: %f\n", driver::storage.getFS().totalBytes() / (1024.f * 1024.f));
        Serial.printf("Storage used bytes: %f\n", driver::storage.getFS().usedBytes() / (1024.f * 1024.f));
    }

#ifndef NO_VIDEO
    // tft test text
    driver::display.fillScreen(TFT_NAVY);
    driver::display.setCursor(8, 8);
    driver::display.setTextColor(TFT_YELLOW);
    driver::display.setTextSize(1);
    driver::display.print("Hello World!");
#if 1
    sprite.createSprite(240, 120);
    service::audioPlayer.setFFTHandler(&spectrum);
#endif
#endif    

#ifndef NO_AUDIO
#if 0
    // start the audio player
    // String format = "mp3", filelist = "Juno Dreams/bad";
    // service::audioPlayer.getUI().playStorage(format, filelist);
#endif
#endif
}

void loop() 
{
    // update hardware
    // driver::ledAndButton.update();

    // update services
    services::update();

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

#ifndef NO_VIDEO
#if 1
    if ((millis() - lastDraw) >= 17)
    {
        spectrum.renderOn(sprite, 1, 0.3f);
        sprite.pushSprite(0, 100);
        lastDraw = millis();
    }
    
#else

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
#endif
#endif
}
