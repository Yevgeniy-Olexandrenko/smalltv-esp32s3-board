#pragma once

#define USE_AUDIOTOOLS_NS false

#include <AudioTools.h>
#include <FS.h>

namespace service::audio_player
{
    class AudioContext
    {
        using onPlaylistItemCallback = void (*)(const char* str, int len);

    public:
        AudioContext() = default;
        virtual ~AudioContext() = default;

        virtual void begin() = 0;
        virtual void end() = 0;

        void setOnPlaylistItemCallback(onPlaylistItemCallback callback) { m_plistItemCB = callback; };

        audio_tools::AudioSource& getSource() { return *m_source; }
        audio_tools::AudioDecoder& getDecoder() { return *m_decode; }

    protected:
        audio_tools::AudioSource* m_source = nullptr;
        audio_tools::AudioDecoder* m_decode = nullptr;
        onPlaylistItemCallback m_plistItemCB = nullptr;
    };

    ////////////////////////////////////////////////////////////////////////////

    class StorageAudioContext : public AudioContext
    {
        static StorageAudioContext* s_this;
        static Stream* s_nextStreamCallback(int offset);

        using Filelist = std::vector<String>;

    public:
        StorageAudioContext(const String& ext, const String& dir, bool shuffle, bool loop);
        ~StorageAudioContext() override;

        void begin() override;
        void end() override;

    private:
        Stream* nextStreamCallback(int offset);
        bool updateListIndex(int offset);

    private:
        audio_tools::AudioSource* m_source;
        audio_tools::AudioDecoder* m_decode;
        audio_tools::AudioDecoder* m_filter;

        String m_path;
        Filelist m_list;
        int16_t m_index;
        bool m_loop;
        fs::File m_file;
    };

    ////////////////////////////////////////////////////////////////////////////

    class RadioAudioContext : public AudioContext
    {
        //
    };
}
