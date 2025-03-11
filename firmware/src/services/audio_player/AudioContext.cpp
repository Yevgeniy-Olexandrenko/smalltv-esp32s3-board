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

    StorageAudioContext::StorageAudioContext(const String& ext, const String& dir, bool shuffle)
        : m_index(-1)
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
                for (; m_list.size() < m_list.capacity();)
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
            }

            if (shuffle)
            {
                std::mt19937 randomGenerator(millis());
                std::shuffle(m_list.begin(), m_list.end(), randomGenerator);
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
        if (!m_source)
        {
            m_cbSrc = new audio_tools::AudioSourceCallback(&s_nextStreamCallback);
            m_source = m_cbSrc;
        }

        if (!m_decode)
        {
            m_codec  = new audio_tools::MP3DecoderHelix();
            m_mdFlt  = new audio_tools::MetaDataFilterDecoder(*m_codec);
            m_decode = m_mdFlt;
        }
    }

    void StorageAudioContext::end()
    {
        if (m_source)
        {
            delete m_cbSrc;
            m_source = nullptr;
        }

        if (m_decode)
        {
            delete m_codec;
            delete m_mdFlt;
            m_decode = nullptr;
        }
    }

    Stream* StorageAudioContext::nextStreamCallback(int offset)
    {
        if (!m_list.empty())
        {
            if ((m_index += offset) < 0)
                m_index += m_list.size();
            else
                m_index %= m_list.size();

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

        log_i("no files to open");
        return nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////
}
