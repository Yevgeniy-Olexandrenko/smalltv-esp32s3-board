#include "AudioPlayer.h"
#include "drivers/storage/Storage.h"
#include "board.h"

namespace service
{
    void AudioPlayer::begin(float volume)
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

        m_cmdQueue = xQueueCreate(8, sizeof(Command));
        setVolume(volume);
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
                path += "/" + String(content.extOrContentType);
                path += "/" + String(resource);
                m_path = path;
                log_i("path: %s", path.c_str());

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

    void AudioPlayer::setVolume(float volume)
    {
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
        if (m_isStarted)
        {
            b.Label("title"_h, "Title", m_title);
            b.Label("artist"_h, "Artist", m_artist);

            {
                sets::Buttons buttons(b);
                b.Button("stop"_h, "Stop");
                b.Button("prev"_h, "Prev");
                b.Button("next"_h, "Next");
                b.Button("play"_h, m_isPlaying ? "Pause" : "Play");

                if (b.build.isAction())
                {
                    switch (b.build.id)
                    {
                        case "stop"_h: stop(); break;
                        case "prev"_h: next(false); break;
                        case "next"_h: next(true); break;
                        case "play"_h: pause(m_isPlaying); break;
                    }
                }
            }
        }
        else
        {
            String typeChoice = "mp3;acc;mod";
            String listChoise = "Free;Jazz;Retrowave;Big;Instrumental";

            if (b.Select("Type", typeChoice))
            {
                // TODO
            }

            if (b.Select("Playlist", listChoise))
            {
                // TODO
            }

            if (b.Button("Start"))
            {
                // TODO
                start(service::StorageFileMP3(), "Christmas");
                b.reload();
            }
        }
    }

    void AudioPlayer::settingsUpdate(sets::Updater &u)
    {
        bool reload = false;
        if (m_isStarted != isStarted())
        {
            m_isStarted ^= true;
            reload = true;
        }
        if (m_isPlaying != isPlaying())
        {
            m_isPlaying ^= true;
            reload = true;
        }

        if (reload)
        {
            settings::sets().reload();
        }
        else
        {
            u.update("title"_h, m_title);
            u.update("artist"_h, m_artist);
        }
    }

    ////////////////////////////////////////////////////////////////////////////

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
        audioPlayer.m_fileIndex = 0;
        audioPlayer.m_dir = driver::storage.getFS().open(audioPlayer.m_path);
        log_i("open dir: %s (%d)", audioPlayer.m_dir.path(), int(audioPlayer.m_dir));
    }

    Stream* AudioPlayer::nextStreamCallback(int offset)
    {
        audioPlayer.m_title.clear();
        audioPlayer.m_artist.clear();

        audioPlayer.m_file.close();
        audioPlayer.m_fileIndex += offset;
        audioPlayer.m_dir.rewindDirectory();
        for (int i = 0; i <= audioPlayer.m_fileIndex; i++)
            audioPlayer.m_file = audioPlayer.m_dir.openNextFile();

        if (audioPlayer.m_file)
        {
            audioPlayer.fetchTitleAndAuthor(audioPlayer.m_file.name());
            log_i("open file: %s (%d)", audioPlayer.m_file.path(), audioPlayer.m_fileIndex);
            return &audioPlayer.m_file;
        }
        return nullptr;
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
                audioPlayer.m_title = String(str, len);
                break;

            case audio_tools::MetaDataType::Artist:
                audioPlayer.m_artist = String(str, len);
                break;
        }
    }

    void AudioPlayer::fetchTitleAndAuthor(String metadata)
    {
        auto i0 = metadata.lastIndexOf('.');
        if (i0 < 0) i0 = metadata.length();

        auto i1 = metadata.indexOf(" - ");
        if (i1 > 0)
        {
            m_title = metadata.substring(i1 + 3, i0);
            m_artist = metadata.substring(0, i1);
        }
        else
            m_title = metadata.substring(0, i0);
    }

    AudioPlayer audioPlayer;
}
