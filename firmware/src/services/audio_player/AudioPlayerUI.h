#pragma once
#ifndef NO_AUDIO

#include "shared/settings/Settings.h"

namespace service::audio_player
{
    class AudioPlayerUI : public settings::Provider
    {
        struct UIList
        {
            std::vector<String> items;
            uint8_t index = 0;

            bool empty() const { return items.empty(); }
            void clear() { items.clear(); index = 0; }
            void sort() { std::sort(items.begin(), items.end()); index = 0; }
            const String& item() const { return items[index]; }
        };

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
        void fillSourcesOptions(String& output);
        void fillFilelistsOptions(String& output);
        void fillPlaylistOptions(String& output);

    private:
        bool m_started;
        bool m_playing;
        UIList m_sources;
        UIList m_filelists;
        bool m_filelistsUpd;
        String m_title;
        String m_artist;
    };
}
#endif
