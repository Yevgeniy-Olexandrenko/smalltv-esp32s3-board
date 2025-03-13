#include "AudioPlayer.h"
#include "AudioContext.h"
#include "AudioPlayerUI.h"
#include "drivers/storage/Storage.h"

namespace service::audio_player
{
    AudioPlayerUI::AudioPlayerUI()
        : m_started(false)
        , m_playing(false)
        , m_format(0)
        , m_playlist(0)
        , m_shuffle(false)
        , m_loop(false)
    {
    }

    void AudioPlayerUI::settingsBuild(sets::Builder &b)
    {
        sets::Group g(b, "Audio player");
        if (m_started)
        {
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
            String formats;
            fetchFormats(formats);
            if (b.Select("Type", formats, &m_format))
            {
                m_playlist = 0;
                b.reload();
            }

            String filelists;
            fetchPlaylists(Text(formats).getSub(m_format, ';'), filelists);
            if (!filelists.isEmpty())
            {
                b.Select("Playlist", filelists, &m_playlist);
                {
                    sets::Row r(b, "", sets::DivType::Default);
                    b.Switch("Shuffle", &m_shuffle);
                    b.Switch("Loop", &m_loop);
                }
                if (b.Button("Start"))
                {
                    String format = Text(formats).getSub(m_format, ';');
                    String filelist = Text(filelists).getSub(m_playlist, ';');

                    audioPlayer.start(new StorageAudioContext(format, filelist, m_shuffle, m_loop));
                    b.reload();
                }
            }
        }
    }

    void AudioPlayerUI::settingsUpdate(sets::Updater &u)
    {
        bool reload = false;
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
        else
        {
            u.update("title"_h, m_title);
            u.update("artist"_h, m_artist);
        }
    }

    void AudioPlayerUI::fetchFormats(String &output)
    {
        output = "mp3;acc;wav;mod";
    }

    void AudioPlayerUI::fetchPlaylists(const String &format, String &output)
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
}
