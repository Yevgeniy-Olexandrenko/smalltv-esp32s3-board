#ifndef NO_AUDIO

#include "AudioPlayer.h"
#include "board.h"

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

    void AudioPlayer::begin()
    {
        if (!m_i2sOut && !m_fftOut)
        {
            m_ui.begin();
            s_this = this;
            m_play.commands = xQueueCreate(4, sizeof(Command));
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
            fftCfg.length = 2048;
            fftCfg.callback = &s_fftCallback;
            m_fftOut.begin(fftCfg);

            // configure MultiOutput
            m_output.add(m_fftOut);
            m_output.add(m_i2sOut);
        }
    }

    bool AudioPlayer::start(details::AudioContext* context)
    {
        if (!isStarted() && context && !m_context)
        {
            m_context.reset(context);
            return Task::start("audio_player");
        }
        return false;
    }

    void AudioPlayer::setVolume(float volume)
    {
        m_play.mutex.lock();
        m_play.param =  0.1f;
        m_play.param += 0.9f * constrain(volume, 0.f, 1.f);
        m_play.param *= SND_PRE_AMP;
        m_play.mutex.unlock();

        if (isStarted())
        {
            auto command { Command::Volume };
            xQueueSend(m_play.commands, &command, portMAX_DELAY);
        }
        else
        {
            m_play.player.setVolume(m_play.param);
        }
    }

    void AudioPlayer::setPlaylistIndex(int index)
    {
        m_play.mutex.lock();
        m_play.param = index;
        m_play.mutex.unlock();

        if (isStarted())
        {
            auto command { Command::Index };
            xQueueSend(m_play.commands, &command, portMAX_DELAY);
        }
    }

    void AudioPlayer::pause(bool yes)
    {
        if (isStarted())
        {
            if (yes)
            {
                auto command { Command::Pause };
                xQueueSend(m_play.commands, &command, portMAX_DELAY);
            }
            else
            {
                auto command { Command::Resume };
                xQueueSend(m_play.commands, &command, portMAX_DELAY);
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
                xQueueSend(m_play.commands, &command, portMAX_DELAY);
            }
            else
            {
                auto command { Command::Prev };
                xQueueSend(m_play.commands, &command, portMAX_DELAY);
            }
        }
    }

    void AudioPlayer::stop()
    {
        if (isStarted())
        {
            auto command { Command::Stop };
            xQueueSend(m_play.commands, &command, portMAX_DELAY);
        }
    }

    bool AudioPlayer::isStarted()
    {
        task::LockGuard lock(m_play.mutex);
        return Task::isAlive();
    }

    bool AudioPlayer::isPlaying()
    {
        task::LockGuard lock(m_play.mutex);
        return (Task::isAlive() && m_play.player.isActive());
    }

    void AudioPlayer::setFFTHandler(details::FFTHandler* fftHandler)
    {
        task::LockGuard lock(m_fft.mutex);
        if (fftHandler) fftHandler->init(m_fftOut);
        m_fft.handler = fftHandler;
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
            task::LockGuard lock(m_play.mutex);
            m_play.player.setAudioSource(m_context->getSource());
            m_play.player.setDecoder(m_context->getDecoder());
            m_play.player.setOutput(m_output);
        //  m_play.player.setMetadataCallback(&s_metadataCallback);
            m_play.player.setVolumeControl(m_volCtr);
            m_play.player.begin();
        }
        while (m_play.player.getStream() != nullptr)
        {
            Command command;
            if (xQueueReceive(m_play.commands, &command, 0) == pdTRUE)
            {
                if (command == Command::Stop) break;

                task::LockGuard lock(m_play.mutex);   
                switch (command)
                {
                    case Command::Volume: m_play.player.setVolume(m_play.param); break;
                    case Command::Pause:  m_play.player.stop(); break;
                    case Command::Resume: m_play.player.play(); break;
                    case Command::Next:   m_play.player.next(); break;
                    case Command::Prev:   m_play.player.previous(); break;
                    case Command::Index:  m_play.player.setIndex(int(m_play.param)); break;
                }
            }

            m_play.player.copy();
            sleep(1);
        }
        {
            task::LockGuard lock(m_play.mutex);
            m_play.player.end();
            m_context.reset();
        }
    }

    void AudioPlayer::fftCallback(audio_tools::AudioFFTBase& fft)
    {
        task::LockGuard lock(m_fft.mutex);
        if (m_fft.handler) m_fft.handler->update(fft);
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
