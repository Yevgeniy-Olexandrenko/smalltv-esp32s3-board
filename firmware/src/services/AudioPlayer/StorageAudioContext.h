#pragma once
#ifndef NO_AUDIO

#include <FS.h>
#include "AudioContext.h"

namespace service::details
{
    class StorageAudioContext : public AudioContext
    {
        static StorageAudioContext* s_this;
        static Stream* s_nextStreamCallback(int offset);
        static Stream* s_indexStreamCallback(int index);

    public:
        static void fetchStorageExts(std::vector<String>& exts);
        static void fetchStorageFilelistsForExt(const String& ext, std::vector<String>& filelists);

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
}
#endif
