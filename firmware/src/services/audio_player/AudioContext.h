#pragma once

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
        using onNewStreamCb = std::function<void(const char* str, int len)>;

    public:
        AudioContext() : m_source(nullptr), m_decode(nullptr), m_newStreamCb(nullptr) {}
        virtual ~AudioContext() {}

        virtual void begin() = 0;
        virtual void end() = 0;

        void setOnNewStreamCb(onNewStreamCb callback) { m_newStreamCb = callback; };

        audio_tools::AudioSource& getSource() { return *m_source; }
        audio_tools::AudioDecoder& getDecoder() { return *m_decode; }

    protected:
        audio_tools::AudioSource* m_source;
        audio_tools::AudioDecoder* m_decode;
        onNewStreamCb m_newStreamCb;
    };

    ////////////////////////////////////////////////////////////////////////////

    class StorageAudioContext : public AudioContext
    {
        static StorageAudioContext* s_this;
        static Stream* s_nextStreamCallback(int offset);

    public:
        StorageAudioContext(const String& ext, const String& dir, bool shuffle, bool loop);
        ~StorageAudioContext() override;

        void begin() override;
        void end() override;

    private:
        Stream* nextStreamCallback(int offset);
        bool updateListIndex(int offset);

    private:
        std::unique_ptr<audio_tools::AudioSource > m_source;
        std::unique_ptr<audio_tools::AudioDecoder> m_decode;
        std::unique_ptr<audio_tools::AudioDecoder> m_filter;

        String m_path;
        std::vector<String> m_list;
        int16_t m_index;
        bool m_loop;
        File m_file;
    };

    ////////////////////////////////////////////////////////////////////////////

    class RadioAudioContext : public AudioContext
    {
        //
    };
}
