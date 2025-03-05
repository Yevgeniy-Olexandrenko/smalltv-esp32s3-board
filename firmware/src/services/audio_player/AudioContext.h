#pragma once

#define USE_AUDIOTOOLS_NS false

#include <AudioTools.h>
#include <FS.h>

namespace service_audio_player_impl
{
    class AudioContext
    {
    public:
        AudioContext() = default;
        virtual ~AudioContext() = default;

        virtual void begin() = 0;
        virtual void end() = 0;

        using MetadataCallback = void (*)(audio_tools::MetaDataType type, const char* str, int len);
        void setMetadataCallback(MetadataCallback callback) { m_mdcb = callback; };

        virtual audio_tools::AudioSource& getSource() = 0;
        virtual audio_tools::AudioDecoder& getDecoder() = 0;

    protected:
        void fetchTitleAndAuthor(String metadata);

    private:
        MetadataCallback m_mdcb = nullptr;
    };

    class StorageAudioContext : public AudioContext
    {
        static StorageAudioContext* s_this;
        static void s_initStreamCallback();
        static Stream* s_nextStreamCallback(int offset);

    public:
        StorageAudioContext(const char* ext, const char* dir);

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
        int m_fileIndex;
        fs::File m_dir;
        fs::File m_file;
    };

    class RadioAudioContext : public AudioContext
    {
        //
    };
}
