#include "AudioPlayer.h"
#include "drivers/storage/Storage.h"
#include "board.h"

void callbackInit();
Stream* callbackNextStream(int offset);

AudioInfo info(44100, 2, 16);
I2SStream i2s;

//MP3DecoderHelix decoder;
MP3DecoderHelix mp3;                     // Decoder
MetaDataFilterDecoder decoder(mp3); // Decoder which removes metadata

AudioSourceCallback source(callbackNextStream, callbackInit);
AudioPlayer player(source, i2s, decoder);

File audioFile;
File dir;

void callbackInit() 
{
    // make sure that the directory contains only mp3 files
    dir = driver::storage.getFS().open("/audio/mp3/pl00");
}
  
Stream* callbackNextStream(int offset) 
{
    audioFile.close();

    // the next files must be a mp3 file
    for (int j = 0; j < offset; j++)
    {
        audioFile = dir.openNextFile();
    }
    return &audioFile;
}

void callbackPrintMetaData(MetaDataType type, const char* str, int len)
{
    Serial.print("==> ");
    Serial.print(toStr(type));
    Serial.print(": ");
    Serial.println(str);
}

#define USE_THREAD 1

namespace service
{
    void AudioPlayer::begin()
    {
#if USE_THREAD
        xTaskCreatePinnedToCore(
            [](void* data) 
            {
                auto instance = static_cast<AudioPlayer*>(data);
                instance->task();
            },
            "audio_player_task", 1024 * 10, this, 1, nullptr, 1
        );
#else
        taskBegin();
#endif
    }

    void AudioPlayer::loop()
    {
#if !USE_THREAD
        taskLoop();
#endif
    }

    void AudioPlayer::task()
    {
        taskBegin();
        while(true)
        {
            taskLoop();
        }
    }

    void AudioPlayer::taskBegin()
    {
        //AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);

        // setup output
        auto cfg = i2s.defaultConfig(TX_MODE);
        cfg.copyFrom(info); 
        cfg.pin_ws = PIN_SND_RLCLK;
        cfg.pin_bck = PIN_SND_BCLK;
        cfg.pin_data = PIN_SND_DIN;
        i2s.begin(cfg);

        // setup player
        player.setMetadataCallback(callbackPrintMetaData);
        player.setAudioInfo(info);
        player.setVolume(0.25f);
        player.begin();
    }

    void AudioPlayer::taskLoop()
    {
        player.copy();
    }

    AudioPlayer audioPlayer;
}
