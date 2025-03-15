#include <random>
#include <AudioTools.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/CodecAACHelix.h>
#include "AudioContext.h"
#include "drivers/storage/Storage.h"

namespace service::audio_player
{
    StorageAudioContext* StorageAudioContext::s_this = nullptr;
    
    Stream* StorageAudioContext::s_nextStreamCallback(int offset)
    {
        return (s_this ? s_this->nextStreamCallback(offset) : nullptr);
    }

    StorageAudioContext::StorageAudioContext(const String& ext, const String& dir, bool shuffle, bool loop)
        : AudioContext()
        , m_index(-1)
        , m_loop(loop)
    {
        s_this = this;
        if (!ext.isEmpty() && !dir.isEmpty())
        {
            auto strExt = ext;
            strExt.toLowerCase();

            m_path = "/audio/" + strExt + "/" + dir;
            File file = driver::storage.getFS().open(m_path);

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
    }

    void StorageAudioContext::begin()
    {
        #ifndef NO_SOUND
        if (!AudioContext::m_source && !AudioContext::m_decode)
        {
            // audio source
            m_source = std::make_unique<AudioSourceCallback>(&s_nextStreamCallback);
            AudioContext::m_source = m_source.get();

            // decode chain
            m_decode = std::make_unique<MP3DecoderHelix>();
            m_filter = std::make_unique<MetaDataFilterDecoder>(*m_decode);
            AudioContext::m_decode = m_filter.get();
        }
        #endif
    }

    void StorageAudioContext::end()
    {
        #ifndef NO_SOUND
        AudioContext::m_source = nullptr;
        AudioContext::m_decode = nullptr;
        m_source.reset();
        m_decode.reset();
        m_filter.reset();
        #endif
    }

    Stream* StorageAudioContext::nextStreamCallback(int offset)
    {
        if (!m_list.empty() && updateListIndex(offset))
        {
            log_i("[%d / %d] try to open", m_index, m_list.size());
            auto path = m_path + "/" + m_list[m_index];
            m_file = driver::storage.getFS().open(path);

            if (m_file)
            {
                String name { m_file.name() };
                auto len = name.lastIndexOf('.');
                if (len < 0) len = name.length();
                m_newStreamCb(name.c_str(), len);

                log_i("[%d / %d] open success: %s", m_index, m_list.size(), m_file.path());
                return &m_file;
            }
        }

        log_i("[%d of %d] open fail", m_index, m_list.size());
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
