#ifndef NO_AUDIO

#include "AudioPlayerUI.h"
#include "AudioContext.h"
#include "services/AudioPlayer.h"
#include "drivers/storage/Storage.h"
#include "settings.h"

namespace service::details
{
    AudioPlayerUI::AudioPlayerUI()
        : m_started(false)
        , m_playing(false)
        , m_filelistsUpd(false)
    {
    }

    void AudioPlayerUI::begin()
    {
        settings::data().init(db::audio_volume, 50);
        settings::data().init(db::audio_player_shuffle, false);
        settings::data().init(db::audio_player_loop, false);
        onVolumeSettingsChanged();
    }

    void AudioPlayerUI::playStorage(const String &format, const String &filelist)
    {
        bool shuffle = settings::data()[db::audio_player_shuffle];
        bool loop = settings::data()[db::audio_player_loop];
        auto context = new StorageAudioContext(format, filelist, shuffle, loop);
        audioPlayer.start(context);
    }

    void AudioPlayerUI::playRadio(const String &stations)
    {
        // TODO
    }

    void AudioPlayerUI::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "🎼 Audio player");
        if (m_started)
        {
            m_sources.clear();
            m_filelists.clear();

            String playlist;
            fillPlaylistOptions(playlist);
            int index = audioPlayer.getContext()->getIndex();

            b.beginRow();
            if (b.Select("file"_h, "File", playlist, &index))
            {
                audioPlayer.setPlaylistIndex(index);
            }
            b.endRow();
            
            b.Label("title"_h, "Title", m_title);
            b.Label("artist"_h, "Artist", m_artist);
            {
                sets::Buttons buttons(b);
                b.Button("stop"_h, "Stop");
                b.Button("prev"_h, "Prev");
                b.Button("next"_h, "Next");
                b.Button("play"_h, m_playing ? "Pause" : "Play");

                if (b.build.isAction())
                {
                    switch (b.build.id)
                    {
                        case "stop"_h: audioPlayer.stop(); break;
                        case "prev"_h: audioPlayer.next(false); break;
                        case "next"_h: audioPlayer.next(true ); break;
                        case "play"_h: audioPlayer.pause(m_playing); break;
                    }
                }
            }
        }
        else
        {
            if (m_sources.empty())
            {
                context_utils::fetchStorageExts(m_sources.items);
                m_sources.sort();
            }
            if (m_filelists.empty())
            {
                m_filelistsUpd = true;
            }

            String sources;
            fillSourcesOptions(sources);
            if (b.Select("Source", sources, &m_sources.index))
            {
                m_filelists.clear();
                b.reload();
            }
            if (!m_filelists.empty())
            {
                String filelists;
                fillFilelistsOptions(filelists);
                b.Select("Playlist", filelists, &m_filelists.index);
            }
            {
                sets::Row r(b, "", sets::DivType::Default);
                b.Switch(db::audio_player_shuffle, "Shuffle");
                b.Switch(db::audio_player_loop, "Loop");
            }
            if (!m_filelists.empty() && b.Button("Play"))
            {
                playStorage(m_sources.item(), m_filelists.item());
            }
        }
    }

    void AudioPlayerUI::settingsUpdate(sets::Updater &u)
    {
        bool reload = false;
        if (m_filelistsUpd)
        {
            m_filelistsUpd = false;
            if (!m_sources.empty())
            {
                const auto& source = m_sources.item();
                context_utils::fetchStorageFilelistsForExt(source, m_filelists.items);
                reload = !m_filelists.empty();
                m_filelists.sort();
            }
        }
        if (m_started != audioPlayer.isStarted())
        {
            m_started ^= true;
            reload = true;
        }
        if (m_playing != audioPlayer.isPlaying())
        {
            m_playing ^= true;
            reload = true;
        }
        if (reload)
        {
            settings::sets().reload();
        }
        else if (m_started)
        {
            auto index = audioPlayer.getContext()->getIndex();
            u.update("file"_h, index);
            u.update("title"_h, m_title);
            u.update("artist"_h, m_artist);
        }
    }

    void AudioPlayerUI::settingsBuildVolume(sets::Builder &b)
    {
        if (b.Slider(db::audio_volume, "🔈 Volume", 0, 100))
        {
            onVolumeSettingsChanged();
        }
    }

    void AudioPlayerUI::onVolumeSettingsChanged()
    {
        auto volume = (float(settings::data()[db::audio_volume]) * 0.01f);
        audioPlayer.setVolume(volume);
    }

    void AudioPlayerUI::fillSourcesOptions(String &output)
    {
        for (auto item : m_sources.items)
        {
            item.toLowerCase();
            output += "Storage: ";
            output += item;
            output += ';';
        }
    }

    void AudioPlayerUI::fillFilelistsOptions(String &output)
    {
        char letter = 0;
        for (auto item : m_filelists.items)
        {
            item.replace('[', '(');
            item.replace(']', ')');
            if (item[0] != letter)
            {
                letter = item[0];
                output += '[';
                output += letter;
                output += ']';
            }
            output += "💿 " + item;
            output += ';';
        }
    }

    void AudioPlayerUI::fillPlaylistOptions(String &output)
    {
        const auto& playlist = audioPlayer.getContext()->getPlaylist();
        const auto  size = playlist.size(); 

        for (int i = 0; i < size; ++i)
        {
            if (size > 10 && (i % 10) == 0)
            {
                output += "[" + String(i + 1) + " / " + String(size) + "]";
            }

            auto item = playlist[i];
            item.replace('[', '(');
            item.replace(']', ')');
            output += "🎶 " + item + ";";
        }
    }
}
#endif
