#ifndef NO_AUDIO

#include "AudioPlayer.h"
#include "shared/tasks/Task.h"

namespace service
{
    AudioPlayer* AudioPlayer::s_this = nullptr;

    void AudioPlayer::s_fftCallback(audio_tools::AudioFFTBase &fft)
    {
        if (s_this) s_this->fftCallback(fft);
    }

    void AudioPlayer::s_metadataCallback(audio_tools::MetaDataType type, const char *str, int len)
    {
        if (s_this) s_this->metadataCallback(type, String(str, len));
    }

    AudioPlayer::AudioPlayer()
        : m_cmdQueue(nullptr)
        , m_handle(nullptr)
        , m_volume(0.f)
    {
    }

    void AudioPlayer::begin()
    {
        if (!m_i2sOut && !m_fftOut)
        {
            m_ui.begin();
            s_this = this;
            m_cmdQueue = xQueueCreate(4, sizeof(Command));
            audio_tools::AudioToolsLogger.begin(Serial, audio_tools::AudioToolsLogLevel::Warning);

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
            fftCfg.callback = &s_fftCallback;
            m_fftOut.begin(fftCfg);

            // configure MultiOutput
            m_output.add(m_fftOut);
            m_output.add(m_i2sOut);
        }
    }

    bool AudioPlayer::start(audio_player::AudioContext* context)
    {
        if (!isStarted() && context && !m_context)
        {
            m_context.reset(context);
            return xTaskCreatePinnedToCore(
                [](void* data) 
                {
                    auto instance = static_cast<AudioPlayer*>(data);
                    instance->task();
                },
                "audio_player", 8192 * 2, this, task::priority::Realtime,
                &m_handle, task::core::Application
            ) == pdPASS;
        }
        return false;
    }

    void AudioPlayer::setVolume(float volume)
    {
        m_mutex.lock();
        m_volume =  0.1f;
        m_volume += 0.9f * constrain(volume, 0.f, 1.f);
        m_volume *= SND_PRE_AMP;
        m_mutex.unlock();

        if (isStarted())
        {
            auto command { Command::Volume };
            xQueueSend(m_cmdQueue, &command, portMAX_DELAY);
        }
    }

    void AudioPlayer::setPlaylistIndex(int index)
    {
        m_mutex.lock();
        m_index = index;
        m_mutex.unlock();

        if (isStarted())
        {
            auto command { Command::Index };
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
        return (m_handle != nullptr);
    }

    bool AudioPlayer::isPlaying()
    {
        task::LockGuard lock(m_mutex);
        return (m_handle != nullptr && m_player.isActive());
    }

    void AudioPlayer::task()
    {
        m_context->begin();
        m_context->setOnNewStreamCb([this](const char *str, int len)
        {
            m_ui.setTitle ("<unknown>");
            m_ui.setArtist("<unknown>");
        });
        {
            task::LockGuard lock(m_mutex);
            m_player.setAudioSource(m_context->getSource());
            m_player.setDecoder(m_context->getDecoder());
            m_player.setOutput(m_output);
            // m_player.setMetadataCallback(&s_metadataCallback);
            m_player.setVolumeControl(m_volCtr);
            m_player.begin();
            m_player.setVolume(m_volume);
        }
        while (m_player.getStream() != nullptr)
        {
            Command command;
            if (xQueueReceive(m_cmdQueue, &command, 0) == pdTRUE)
            {
                if (command == Command::Stop) break;

                task::LockGuard lock(m_mutex);   
                switch (command)
                {
                    case Command::Volume: m_player.setVolume(m_volume); break;
                    case Command::Pause:  m_player.stop(); break;
                    case Command::Resume: m_player.play(); break;
                    case Command::Next:   m_player.next(); break;
                    case Command::Prev:   m_player.previous(); break;
                    case Command::Index:  m_player.setIndex(m_index); break;
                }
            }

            m_player.copy();
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        {
            task::LockGuard lock(m_mutex);
            m_player.end();
            m_context.reset();
            m_handle = nullptr;
        }
        vTaskDelete(nullptr);
    }

    void AudioPlayer::fftCallback(audio_tools::AudioFFTBase &fft)
    {
        // TODO
    }

    void AudioPlayer::metadataCallback(audio_tools::MetaDataType type, const String& str)
    {
        switch (type)
        {
            case audio_tools::MetaDataType::Title: m_ui.setTitle(str); break;
            case audio_tools::MetaDataType::Artist: m_ui.setArtist(str); break;
        }
    }

    AudioPlayer audioPlayer;
}
#endif
