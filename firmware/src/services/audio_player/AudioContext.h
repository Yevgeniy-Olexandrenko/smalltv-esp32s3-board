#pragma once

#define USE_AUDIOTOOLS_NS false

#include <AudioTools.h>
#include <FS.h>

namespace service_audio_player_impl
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

    class StorageAudioContext : public AudioContext
    {
        static StorageAudioContext* s_this;
        static Stream* s_nextStreamCallback(int offset);

    public:
        StorageAudioContext(const String& ext, const String& dir, bool shuffle);
        ~StorageAudioContext() override;

        void begin() override;
        void end() override;

    private:
        Stream* nextStreamCallback(int offset);

    private:
        audio_tools::AudioSourceCallback* m_cbSrc;
        audio_tools::MetaDataFilterDecoder* m_mdFlt;
        audio_tools::AudioDecoder* m_codec;

        String m_path;
        std::vector<String> m_list;
        int16_t m_index;
        fs::File m_file;
    };

    class RadioAudioContext : public AudioContext
    {
        //
    };
}
