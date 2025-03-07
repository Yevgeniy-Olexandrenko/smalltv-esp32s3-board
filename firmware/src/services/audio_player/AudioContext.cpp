#include "AudioContext.h"
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/CodecAACHelix.h>
#include "drivers/storage/Storage.h"

namespace service_audio_player_impl
{
    StorageAudioContext* StorageAudioContext::s_this = nullptr;

    void StorageAudioContext::s_initStreamCallback() 
    { 
        if (s_this) s_this->initStreamCallback(); 
    }
    
    Stream* StorageAudioContext::s_nextStreamCallback(int offset)
    {
        return (s_this ? s_this->nextStreamCallback(offset) : nullptr);
    }

    StorageAudioContext::StorageAudioContext(const char* ext, const char* dir)
        : m_source(nullptr)
        , m_decode(nullptr)
    {
        s_this = this;
        if (ext && dir)
        {
            String path;
            path += "/audio";
            path += "/" + String(ext);
            path += "/" + String(dir);
            log_i("path: %s", path.c_str());
            m_path = path;
        }
    }

    StorageAudioContext::~StorageAudioContext()
    {
        end();
        m_file.close();
        m_dir.close();
    }

    void StorageAudioContext::begin()
    {
        if (!m_source)
        {
            m_cbSrc = new audio_tools::AudioSourceCallback();
            m_cbSrc->setCallbackOnStart(&s_initStreamCallback);
            m_cbSrc->setCallbackNextStream(&s_nextStreamCallback);
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

    void StorageAudioContext::initStreamCallback()
    {
        m_idx = -1;
        m_dir = driver::storage.getFS().open(m_path);
        log_i("open dir: %s", m_dir.path());
    }

    Stream* StorageAudioContext::nextStreamCallback(int offset)
    {
        m_file.close();
        m_idx += offset;

        String path;
        m_dir.rewindDirectory();
        for (int i = 0; i <= m_idx; i++) path = m_dir.getNextFileName();

        if (!path.isEmpty())
        {
            m_file = driver::storage.getFS().open(path);
            if (m_file)
            {
                String name = m_file.name();
                auto len = name.lastIndexOf('.');
                if (len < 0) len = name.length();
                m_plistItemCB(name.c_str(), len);

                log_i("open file: %s (%d)", m_file.path(), m_idx);
                return &m_file;
            }
        }

        log_i("no files to open");
        return nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////
}
