#pragma once
#ifndef NO_AUDIO

#include <vector>
#include <functional>
#include <FS.h>

namespace audio_tools
{
    class AudioSource;
    class AudioDecoder;
}

namespace service::audio_player
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

        bool updatePlayistIndex(int offset);
        bool setPlayistIndex(int index);

    protected:
        audio_tools::AudioSource* m_source;
        audio_tools::AudioDecoder* m_decode;
        onNewStreamCb m_newStreamCb;
        Playlist m_playlist;
        int16_t m_direction;
        int16_t m_index;
        bool m_loop;
    };

    ////////////////////////////////////////////////////////////////////////////

    class StorageAudioContext : public AudioContext
    {
        static StorageAudioContext* s_this;
        static Stream* s_nextStreamCallback(int offset);
        static Stream* s_indexStreamCallback(int index);

    public:
        StorageAudioContext(const String& ext, const String& dir, bool shuffle, bool loop);
        ~StorageAudioContext() override;

        void begin() override;
        void end() override;

    private:
        Stream* nextStreamCallback(int offset);
        Stream* indexStreamCallback(int index);
        Stream* openPlaylistItemStream();

    private:
        std::unique_ptr<audio_tools::AudioSource > m_source;
        std::unique_ptr<audio_tools::AudioDecoder> m_decode;
        std::unique_ptr<audio_tools::AudioDecoder> m_filter;

        String m_path;
        File m_file;
    };

    ////////////////////////////////////////////////////////////////////////////

    class RadioAudioContext : public AudioContext
    {
        //
    };
}
#endif
