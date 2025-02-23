#include "AudioPlayer.h"
#include "drivers/storage/Storage.h"
#include "board.h"

namespace service
{
    void AudioPlayer::begin(float vol)
    {
        audio_tools::AudioToolsLogger.begin(Serial, audio_tools::AudioToolsLogLevel::Error);

        if (!m_i2sOut && !m_fftOut)
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
        volume(vol);
    }

    bool AudioPlayer::start(AudioContent content, const char* resource)
    {
        if (!isStarted() && resource)
        {
            if (content.format == AudioContent::Format::mp3 && 
                content.location == AudioContent::Location::StorageFile)
            {
                // prepare source and decoder
                String path;
                path += "/audio";
                path += '/' + content.extOrContentType;
                path += '/' + resource;
                m_path = path;

                // launch playback task
                return xTaskCreatePinnedToCore(
                    [](void* data) 
                    {
                        auto instance = static_cast<AudioPlayer*>(data);
                        instance->task();
                    },
                    "audio_player_task", 8192, this, 1, &m_task.handle, 1
                ) == pdPASS;
            }
        }
        return false;
    }

    void AudioPlayer::volume(float vol)
    {
        task::LockGuard lock(m_mutex);

        m_task.volume =  0.2f;
        m_task.volume += 0.8f * constrain(vol, 0.f, 1.f);
        m_task.volume *= SND_PRE_AMP;

        if (isStarted())
        {
            auto command { Command::Volume };
            xQueueSend(m_cmdQueue, &command, portMAX_DELAY);
        }
    }

    void AudioPlayer::pause(bool yes)
    {
        if (isStarted())
        {
            if (yes)
            {
                auto command { Command::Pause };
                xQueueSend(m_cmdQueue, &command, portMAX_DELAY);
            }
            else
            {
                auto command { Command::Resume };
                xQueueSend(m_cmdQueue, &command, portMAX_DELAY);
            }
        }
    }

    void AudioPlayer::next(bool fwd)
    {
        if (isStarted())
        {
            if (fwd)
            {
                auto command { Command::Next };
                xQueueSend(m_cmdQueue, &command, portMAX_DELAY);
            }
            else
            {
                auto command { Command::Prev };
                xQueueSend(m_cmdQueue, &command, portMAX_DELAY);
            }
        }
    }

    void AudioPlayer::stop()
    {
        if (isStarted())
        {
            auto command { Command::Stop };
            xQueueSend(m_cmdQueue, &command, portMAX_DELAY);
        }
    }

    bool AudioPlayer::isStarted()
    {
        task::LockGuard lock(m_mutex);
        return (m_task.handle != nullptr);
    }

    bool AudioPlayer::isPlaying()
    {
        bool taskStarted = isStarted();
        return (taskStarted && m_task.player.isActive());
    }

    void AudioPlayer::task()
    {
        {   task::LockGuard lock(m_mutex);
            initSource();
            initDecode();

            m_task.player.setAudioSource(*m_source);
            m_task.player.setDecoder(*m_decode);
            m_task.player.setOutput(m_output);

            m_task.player.setMetadataCallback(metadataCallback);
            m_task.player.setVolumeControl(m_volCtr);
            m_task.player.setVolume(m_task.volume);

            m_task.player.begin();
        }

        while (true)
        {
            Command command;
            if (xQueueReceive(m_cmdQueue, &command, 0) == pdTRUE)
            {
                if (command == Command::Stop) break;

                task::LockGuard lock(m_mutex);
                switch (command)
                {
                    case Command::Volume:
                        m_task.player.setVolume(m_task.volume);
                        break;

                    case Command::Pause:
                        m_task.player.setActive(false);
                        break;

                    case Command::Resume:
                        m_task.player.setActive(true);
                        break;

                    case Command::Next:
                        m_task.player.next();
                        break;

                    case Command::Prev:
                        m_task.player.previous();
                        break;
                }
            }
            m_task.player.copy();
        }

        {   task::LockGuard lock(m_mutex);
            m_task.handle = nullptr;
            m_task.player.end();

            deinitDecode();
            deinitSource();
        }
        vTaskDelete(nullptr);
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

    void AudioPlayer::deinitSource()
    {
        delete m_cbSrc;
        m_cbSrc = nullptr;
        m_source = nullptr;
    }

    void AudioPlayer::deinitDecode()
    {
        delete m_mp3Dec;
        delete m_id3Flt;
        m_mp3Dec = nullptr;
        m_id3Flt = nullptr;
        m_decode = nullptr;
    }

    void AudioPlayer::initStreamCallback()
    {
        audioPlayer.m_dir = driver::storage.getFS().open(audioPlayer.m_path);
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
