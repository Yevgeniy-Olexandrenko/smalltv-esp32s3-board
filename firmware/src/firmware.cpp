#include <Arduino.h>
#include "defines.h"

// hardware drivers
#include "drivers/LedAndButton.h"
#include "drivers/onboard/PowerSource.h"
#include "drivers/storage/Storage.h"

// background services
#include "services/network-connection/NetworkConnection.h"
#include "services/geo-location/GeoLocation.h"
#include "services/date-and-time/DateAndTime.h"
#include "services/weather-forecast/Weather.h"

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

void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
    (void)cbData;
    Serial.printf("ID3 callback for: %s = '", type);

    if (isUnicode)
    {
        string += 2;
    }

    while (*string)
    {
        char a = *(string++);
        if (isUnicode)
        {
            string++;
        }
        Serial.printf("%c", a);
    }
    Serial.printf("'\n");
    Serial.flush();
}

#if 1
#include "shared/audio/source/SourceFile.h"
#include "shared/audio/source/SourceMemory.h"
#include "shared/audio/source/SourceExtractID3.h"
#include "shared/audio/decode/DecodeMOD.h"
#include "shared/audio/decode/DecodeMP3.h"
#include "shared/audio/output/OutputI2S.h"

#define PLAY_MP3

audio::Source* source = nullptr;
audio::Source* filter = nullptr;
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
    #define FILTER_P static_cast<audio::SourceMemory*>(source)
#endif

#ifdef PLAY_MP3
    source = new audio::SourceFile();
    filter = new audio::SourceExtractID3(source);
    decode = new audio::DecodeMP3();
    decode->setCallback(MDCallback, (void*)"ID3TAG");
    dir = driver::storage.getFS().open("/audio/mp3");

    #define FILE_EXT ".mp3"
    #define SOURCE_P static_cast<audio::SourceFile*>(source)
    #define FILTER_P static_cast<audio::SourceExtractID3*>(filter)
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
                    decode->begin(FILTER_P, output);
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
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3a.h>
#include <AudioOutputI2S.h>

AudioFileSourceFS* source = nullptr;
AudioFileSourceID3* filter = nullptr;
AudioGeneratorMP3a* decode = nullptr;
AudioOutputI2S* output = nullptr;

fs::File dir;
bool s_forceNext = false;

void sound_setup()
{
    //audioLogger = &Serial;
    source = new AudioFileSourceFS(driver::storage.getFS());
    filter = new AudioFileSourceID3(source);
    decode = new AudioGeneratorMP3a();
    output = new AudioOutputI2S();

    filter->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
    dir = driver::storage.getFS().open("/audio/mp3");
    
    output->SetPinout(PIN_SND_BCLK, PIN_SND_RLCLK, PIN_SND_DIN);
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

            if (!filename.startsWith(".") && filename.endsWith(".mp3"))
            {
                source->close();
                if (source->open(file.path()))
                {
                    Serial.printf("Playing file: %s\n", file.path());
                    decode->begin(filter, output);
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
