#include <random>
#include "AudioContext.h"
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/CodecAACHelix.h>
#include "drivers/storage/Storage.h"

namespace service_audio_player_impl
{
    StorageAudioContext* StorageAudioContext::s_this = nullptr;
    
    Stream* StorageAudioContext::s_nextStreamCallback(int offset)
    {
        return (s_this ? s_this->nextStreamCallback(offset) : nullptr);
    }

    StorageAudioContext::StorageAudioContext(const String& ext, const String& dir, bool shuffle, bool loop)
        : m_source(nullptr)
        , m_decode(nullptr)
        , m_filter(nullptr)
        , m_index(-1)
        , m_loop(loop)
    {
        s_this = this;
        if (!ext.isEmpty() && !dir.isEmpty())
        {
            auto strExt = ext;
            strExt.toLowerCase();

            m_path = "/audio/" + strExt + "/" + dir;
            fs::File file = driver::storage.getFS().open(m_path);

            if (file && file.isDirectory())
            {
                m_list.reserve(256);
                while (m_list.size() < m_list.capacity())
                {
                    auto path = file.getNextFileName();
                    if (path.isEmpty()) break;

                    auto i = path.lastIndexOf('/') + 1;
                    auto strName = path.substring(i);
                    strName.toLowerCase();

                    if (strName[0] != '.' && strName.endsWith('.' + strExt))
                    {
                        m_list.push_back(path.substring(i));
                    }
                }

                if (shuffle)
                {
                    std::mt19937 rnd(millis());
                    std::shuffle(m_list.begin(), m_list.end(), rnd);
                }
            }
        }
    }

    StorageAudioContext::~StorageAudioContext()
    {
        end();
        m_file.close();
        m_list.clear();
    }

    void StorageAudioContext::begin()
    {
        if (!AudioContext::m_source)
        {
            m_source = new audio_tools::AudioSourceCallback(&s_nextStreamCallback);
            AudioContext::m_source = m_source;
        }

        if (!AudioContext::m_decode)
        {
            m_decode = new audio_tools::MP3DecoderHelix();
            m_filter = new audio_tools::MetaDataFilterDecoder(*m_decode);
            AudioContext::m_decode = m_filter;
        }
    }

    void StorageAudioContext::end()
    {
        if (AudioContext::m_source)
        {
            AudioContext::m_source = nullptr;
            delete m_source;
        }

        if (AudioContext::m_decode)
        {
            AudioContext::m_decode = nullptr;
            delete m_decode;
            delete m_filter;
        }
    }

    Stream* StorageAudioContext::nextStreamCallback(int offset)
    {
        if (!m_list.empty() && updateListIndex(offset))
        {
            auto path = m_path + "/" + m_list[m_index];
            m_file = driver::storage.getFS().open(path);

            if (m_file)
            {
                String name { m_file.name() };
                auto len = name.lastIndexOf('.');
                if (len < 0) len = name.length();
                m_plistItemCB(name.c_str(), len);

                log_i("open file: %s (%d of %d)", m_file.path(), m_index, m_list.size());
                return &m_file;
            }
        }

        log_i("no files to open (%d of %d)", m_index, m_list.size());
        return nullptr;
    }

    bool StorageAudioContext::updateListIndex(int offset)
    {
        const auto size = m_list.size();
        const auto next = m_index + offset;

        if (m_loop || (next >= 0 && next < size))
        {
            m_index += offset;
            if (m_index < 0) 
                m_index += size;
            else 
                m_index %= size;
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
}
