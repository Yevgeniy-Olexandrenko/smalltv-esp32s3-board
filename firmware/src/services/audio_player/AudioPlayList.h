#pragma once

#include <FS.h>
#include <vector>

namespace service_audio_player_impl
{
    class AudioPlayList
    {
    public:
        void open(const String& resource, const String& ext);
        String getPrev();
        String getNext();
        void close();

        void shuffle();
        uint8_t getSize() const { return uint8_t(m_list.size()); }
        bool isEmpty() const { return m_list.empty(); }

    private:
        void insertIfValid(const String& path, const String& ext);

    private:
        std::vector<String> m_list;
        uint8_t m_index;
    };
}
