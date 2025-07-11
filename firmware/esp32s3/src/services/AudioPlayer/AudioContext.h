#pragma once
#ifndef NO_AUDIO

#include <vector>
#include <functional>
#include <WString.h>

namespace audio_tools
{
    class AudioSource;
    class AudioDecoder;
}

namespace service::details
{
    class AudioContext
    {
    public:
        using onNewStreamCb = std::function<void(const char* str, int len)>;
        using Playlist = std::vector<String>;

        AudioContext();
        virtual ~AudioContext() = default;

        virtual void begin() = 0;
        virtual void end() = 0;

        void setOnNewStreamCb(onNewStreamCb callback) { m_newStreamCb = callback; };

        audio_tools::AudioSource& getSource() { return *m_source; }
        audio_tools::AudioDecoder& getDecoder() { return *m_decode; }

        const Playlist& getPlaylist() const { return m_playlist; }
        const int16_t&  getDirection() const { return m_direction; }
        const int16_t&  getIndex() const { return m_index; }

        bool updatePlaylistIndex(int16_t offset);
        bool setPlaylistIndex(int16_t index);

    protected:
        audio_tools::AudioSource* m_source;
        audio_tools::AudioDecoder* m_decode;
        onNewStreamCb m_newStreamCb;
        Playlist m_playlist;
        int16_t m_direction;
        int16_t m_index;
        bool m_loop;
    };
}
#endif
