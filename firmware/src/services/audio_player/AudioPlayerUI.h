#pragma once
#ifndef NO_AUDIO

#include "shared/settings/Settings.h"

namespace service::audio_player
{
    class AudioPlayerUI : public settings::Provider
    {
    public:
        AudioPlayerUI();

        void begin();
        void playStorage(const String& format, const String& filelist);
        void playRadio(const String& stations);

        void setTitle(const String& title) { m_title = title; }
        const String& getTitle() const { return m_title; }

        void setArtist(const String& artist) { m_artist = artist; }
        const String& getArtist() const { return m_artist; }

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;
        void settingsBuildVolume(sets::Builder& b);

    private:
        void onVolumeSettingsChanged();
        void fetchFormats(String& output);
        void fetchPlaylists(const String& format, String& output);

    private:
        bool m_started;
        bool m_playing;
        uint8_t m_format;
        uint8_t m_playlist;
        String m_title;
        String m_artist;
    };
}
#endif
