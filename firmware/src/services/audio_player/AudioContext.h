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

        virtual audio_tools::AudioSource& getSource() = 0;
        virtual audio_tools::AudioDecoder& getDecoder() = 0;

    protected:
        onPlaylistItemCallback m_plistItemCB = nullptr;
    };

    class StorageAudioContext : public AudioContext
    {
        static StorageAudioContext* s_this;
        static void s_initStreamCallback();
        static Stream* s_nextStreamCallback(int offset);

    public:
        StorageAudioContext(const char* ext, const char* dir);
        ~StorageAudioContext() override;

        void begin() override;
        void end() override;

        audio_tools::AudioSource& getSource() override { return *m_source; }
        audio_tools::AudioDecoder& getDecoder() override { return *m_decode; }

    private:
        void initStreamCallback();
        Stream* nextStreamCallback(int offset);

    private:
        audio_tools::AudioSource* m_source;
        audio_tools::AudioDecoder* m_decode;

        audio_tools::AudioSourceCallback* m_cbSrc;
        audio_tools::MetaDataFilterDecoder* m_mdFlt;
        audio_tools::AudioDecoder* m_codec;

        String m_path;
        int m_idx;
        File m_dir;
        File m_file;
    };

    class RadioAudioContext : public AudioContext
    {
        //
    };
}
