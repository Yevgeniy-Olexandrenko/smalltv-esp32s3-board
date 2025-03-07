#include "AudioPlayer.h"
#include "drivers/storage/Storage.h"
#include "board.h"

namespace service
{
    void AudioPlayer::begin(float volume)
    {
        #ifndef NO_SOUND
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

        m_cmdQueue = xQueueCreate(8, sizeof(Command));
        setVolume(volume);
        #endif
    }

    bool AudioPlayer::start(AudioContext* context)
    {
        #ifndef NO_SOUND
        if (!isStarted() && context && !m_task.context)
        {
            m_task.context = context;
            return xTaskCreatePinnedToCore(
                [](void* data) 
                {
                    auto instance = static_cast<AudioPlayer*>(data);
                    instance->task();
                },
                "audio_player_task", 8192, this, 1, &m_task.handle, 1
            ) == pdPASS;
        }
        #endif
        return false;
    }

    void AudioPlayer::setVolume(float volume)
    {
        #ifndef NO_SOUND
        m_mutex.lock();
        m_task.volume =  0.1f;
        m_task.volume += 0.9f * constrain(volume, 0.f, 1.f);
        m_task.volume *= SND_PRE_AMP;
        m_mutex.unlock();
        log_i("new volume: %f", m_task.volume);

        if (isStarted())
        {
            auto command { Command::Volume };
            xQueueSend(m_cmdQueue, &command, portMAX_DELAY);
        }
        #endif
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

    ////////////////////////////////////////////////////////////////////////////

    void AudioPlayer::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "Audio player");
        if (m_ui.started)
        {
            b.Label("title"_h, "Title", m_ui.title);
            b.Label("artist"_h, "Artist", m_ui.artist);

            {
                sets::Buttons buttons(b);
                b.Button("stop"_h, "Stop");
                b.Button("prev"_h, "Prev");
                b.Button("next"_h, "Next");
                b.Button("play"_h, m_ui.playing ? "Pause" : "Play");

                if (b.build.isAction())
                {
                    switch (b.build.id)
                    {
                        case "stop"_h: stop(); break;
                        case "prev"_h: next(false); break;
                        case "next"_h: next(true); break;
                        case "play"_h: pause(m_ui.playing); break;
                    }
                }
            }
        }
        else
        {
            String formats;
            fetchFormats(formats);
            if (b.Select("Type", formats, &m_ui.format))
            {
                m_ui.playlist = 0;
                b.reload();
            }

            String filelists;
            fetchPlaylists(Text(formats).getSub(m_ui.format, ';'), filelists);
            if (!filelists.isEmpty())
            {
                b.Select("Playlist", filelists, &m_ui.playlist);
                if (b.Button("Start"))
                {
                    String format = Text(formats).getSub(m_ui.format, ';');
                    String filelist = Text(filelists).getSub(m_ui.playlist, ';');

                    start(new StorageAudioContext(format.c_str(), filelist.c_str()));
                    b.reload();
                }
            }
        }
    }

    void AudioPlayer::settingsUpdate(sets::Updater &u)
    {
        bool reload = false;
        if (m_ui.started != isStarted())
        {
            m_ui.started ^= true;
            reload = true;
        }
        if (m_ui.playing != isPlaying())
        {
            m_ui.playing ^= true;
            reload = true;
        }

        if (reload)
        {
            settings::sets().reload();
        }
        else
        {
            u.update("title"_h, m_ui.title);
            u.update("artist"_h, m_ui.artist);
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    void AudioPlayer::fetchFormats(String &output)
    {
        output = "mp3;acc;wav;mod";
    }

    void AudioPlayer::fetchPlaylists(const String &format, String &output)
    {
        output.clear();
        File dir = driver::storage.getFS().open("/audio/" + format);

        if (dir.isDirectory())
        {
            for (int count = 256; count--;)
            {
                bool isDir = false;
                String entry = dir.getNextFileName(&isDir);
                if (entry.isEmpty()) break;

                if (isDir)
                {
                    output += entry.substring(entry.lastIndexOf('/') + 1);
                    output += ';';
                }
            }
        }
    }

    void AudioPlayer::task()
    {
        {   
            task::LockGuard lock(m_mutex);
            m_task.context->begin();
            m_task.context->setMetadataCallback(&metadataCallback);
            
            m_task.player.setAudioSource(m_task.context->getSource());
            m_task.player.setDecoder(m_task.context->getDecoder());
            m_task.player.setOutput(m_output);

            m_task.player.setMetadataCallback(&metadataCallback);
            m_task.player.setVolumeControl(m_volCtr);
            m_task.player.begin();

            m_task.player.setVolume(m_task.volume);
            log_i("start with volume: %f", m_task.volume);
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
                        log_i("received volume: %f", m_task.volume);
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

        {   
            task::LockGuard lock(m_mutex);
            m_task.player.end();
            m_task.context->end();
            delete m_task.context;
            m_task.context = nullptr;
            m_task.handle = nullptr;
        }
        vTaskDelete(nullptr);
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

        switch (type)
        {
            case audio_tools::MetaDataType::Title:
                audioPlayer.m_ui.title = String(str, len);
                break;

            case audio_tools::MetaDataType::Artist:
                audioPlayer.m_ui.artist = String(str, len);
                break;
        }
    }

    AudioPlayer audioPlayer;
}
