#include "AudioPlayer.h"
#include "drivers/storage/Storage.h"
#include "board.h"

namespace service
{
    AudioPlayer::AudioPlayer()
    {
    }

    AudioPlayer::~AudioPlayer()
    {
    }

    void AudioPlayer::begin(float volume)
    {
        setVolume(volume);
        xTaskCreatePinnedToCore(
            [](void* data) 
            {
                auto instance = static_cast<AudioPlayer*>(data);
                instance->task();
            },
            "audio_player_task", 1024 * 8, this, 1, nullptr, 1
        );
    }

    void AudioPlayer::setVolume(float volume)
    {
        volume = 0.2f + 0.8f * constrain(volume, 0.f, 1.f);
        m_player.setVolume(volume * SND_PRE_AMP);
    }

    void AudioPlayer::task()
    {
        // AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);

        initSource();
        initDecode();
        initOutput();

        m_player.setAudioSource(*m_source);
        m_player.setDecoder(*m_decode);
        m_player.setOutput(m_output);

        m_player.setVolumeControl(m_volume);
        m_player.setMetadataCallback(metadataCallback);
        m_player.begin();

        for (;;)
        {
            // TODO
            m_player.copy();
        }
    }

    void AudioPlayer::initSource()
    {
        m_cbSrc = new audio_tools::AudioSourceCallback();
        m_cbSrc->setCallbackOnStart(&initStreamCallback);
        m_cbSrc->setCallbackNextStream(&nextStreamCallback);
        m_source = m_cbSrc;
    }

    void AudioPlayer::initDecode()
    {
        m_mp3Dec = new audio_tools::MP3DecoderHelix();
        m_id3Flt = new audio_tools::MetaDataFilterDecoder(*m_mp3Dec);
        m_decode = m_id3Flt;
    }

    void AudioPlayer::initOutput()
    {
        // configure I2S
        auto i2sCfg = m_i2sOut.defaultConfig();
        i2sCfg.pin_ws = PIN_SND_RLCLK;
        i2sCfg.pin_bck = PIN_SND_BCLK;
        i2sCfg.pin_data = PIN_SND_DIN;
        m_i2sOut.begin(i2sCfg);

        // configure FFT
        auto fftCfg = m_fftOut.defaultConfig();
        fftCfg.copyFrom(i2sCfg);
        fftCfg.length = 1024;
        fftCfg.callback = &fftResultCallback;
        m_fftOut.begin(fftCfg);

        // configure MultiOutput
        m_output.add(m_fftOut);
        m_output.add(m_i2sOut);
    }

    void AudioPlayer::initStreamCallback()
    {
        const char* path = "/audio/mp3/pl01";
        audioPlayer.m_dir = driver::storage.getFS().open(path);
    }

    Stream* AudioPlayer::nextStreamCallback(int offset)
    {
        audioPlayer.m_file.close();
        for (int i = 0; i < offset; i++)
            audioPlayer.m_file = audioPlayer.m_dir.openNextFile();
        return &audioPlayer.m_file;
    }

    void AudioPlayer::fftResultCallback(audio_tools::AudioFFTBase &fft)
    {
        // float diff;
        // auto result = fft.result();
        // if (result.magnitude>100){
        //     Serial.print(result.frequency);
        //     Serial.print(" ");
        //     Serial.print(result.magnitude);  
        //     Serial.print(" => ");
        //     Serial.print(result.frequencyAsNote(diff));
        //     Serial.print( " diff: ");
        //     Serial.println(diff);
        // }
    }

    void AudioPlayer::metadataCallback(audio_tools::MetaDataType type, const char *str, int len)
    {
        Serial.print("==> ");
        Serial.print(toStr(type));
        Serial.print(": ");
        Serial.println(str);
    }

    AudioPlayer audioPlayer;
}
