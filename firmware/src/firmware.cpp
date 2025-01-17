#include <Arduino.h>
#include "defines.h"
#include "hardware.h"

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

#if 1
#include "audio/source/SourceFile.h"
#include "audio/source/SourceMemory.h"
#include "audio/decode/DecodeMOD.h"
#include "audio/decode/DecodeMP3.h"
#include "audio/output/OutputI2S.h"

#define PLAY_MP3

audio::Source* source = nullptr;
audio::Decode* decode = nullptr;
audio::Output* output = nullptr;

bool s_forceNext = false;
fs::File dir;

void sound_setup()
{
#ifdef PLAY_MOD
    source = new audio::SourceMemory();
    decode = new audio::DecodeMOD();
    dir = driver::storage.getFS().open("/audio/mod");
    #define FILE_EXT ".mod"
    #define SOURCE_P static_cast<audio::SourceMemory*>(source)
#endif

#ifdef PLAY_MP3
    source = new audio::SourceFile();
    decode = new audio::DecodeMP3();
    dir = driver::storage.getFS().open("/audio/mp3");
    #define FILE_EXT ".mp3"
    #define SOURCE_P static_cast<audio::SourceFile*>(source) 
#endif

    output = new audio::OutputI2S();
    static_cast<audio::OutputI2S*>(output)->SetPinout(PIN_SND_BCLK, PIN_SND_RLCLK, PIN_SND_DIN);
    output->SetGain(0.5f);
}

void sound_loop()
{
    if (s_forceNext && decode && decode->isRunning())
    {
        decode->stop();
        s_forceNext = false;
    }

    if (decode && decode->isRunning())
    {
        if (!decode->loop()) decode->stop();
    }
    else
    {
        s_forceNext = false;
        File file = dir.openNextFile();
        if (file)
        {
            String filename(file.name());
            filename.toLowerCase();

            if (!filename.startsWith(".") && filename.endsWith(FILE_EXT))
            {
                source->close();
                if (SOURCE_P->open(driver::storage.getFS(), file.path()))
                {
                    Serial.printf("Playing file: %s\n", file.path());
                    decode->begin(source, output);
                }
                else
                {
                    Serial.printf("Error opening: %s\n", file.path());
                }
            }
        }
    }
}
#else
#include <AudioFileSourceFS.h>
#include <AudioGeneratorMP3a.h>
#include <AudioOutputI2S.h>

AudioFileSourceFS* source = nullptr;
AudioGeneratorMP3a* decoder = nullptr;
AudioOutputI2S* output = nullptr;
fs::File dir;
bool s_forceNext = false;

void sound_setup()
{
    //audioLogger = &Serial;
    source = new AudioFileSourceFS(driver::storage.getFS());
    decoder = new AudioGeneratorMP3a();
    output = new AudioOutputI2S();

    dir = driver::storage.getFS().open("/mp3");
    bool setPinoutOK = output->SetPinout(I2S_BCK_PIN, I2S_WS_PIN, I2S_DO_PIN);
    output->SetGain(0.5f);

    // decoder->SetBufferSize(1024);
    // decoder->SetSampleRate(44100);
    // decoder->SetStereoSeparation(64);

    Serial.printf("openDirOK: %d\n", bool(dir));
    Serial.printf("setPinoutOK: %d\n", setPinoutOK);
}

void sound_loop()
{
    if (s_forceNext && decoder)
    {
        decoder->stop();
        s_forceNext = false;
    }

    if (decoder && decoder->isRunning())
    {
        if (!decoder->loop())
            decoder->stop();
    }
    else
    {
        s_forceNext = false;
        File file = dir.openNextFile();
        if (file)
        {
            String filename(file.name());
            filename.toLowerCase();

            if (!filename.startsWith(".") && filename.endsWith(".mp3"))
            {
                source->close();
                if (source->open(file.path()))
                {
                    Serial.printf("Playing file: %s\n", file.path());
                    decoder->begin(source, output);
                }
                else
                {
                    Serial.printf("Error opening: %s\n", file.path());
                }
            }
        }
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////

void setup() 
{
    Serial.begin(115200);
    delay(1500);

    // turn on the onboard led
    #ifdef PIN_LED_CAT
    pinMode(PIN_LED_CAT, OUTPUT);
    digitalWrite(PIN_LED_CAT, LOW);
    #endif

    // turn on the display backlight
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);

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
            s_forceNext = true;
        }
        else
        {
            Serial.println("Button released!");
        }
    }

    // test sound
    sound_loop();
}
