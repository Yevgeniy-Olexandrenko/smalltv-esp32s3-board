#include "AudioContext.h"
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/CodecAACHelix.h>
#include "drivers/storage/Storage.h"

namespace service_audio_player_impl
{
    void AudioContext::fetchTitleAndAuthor(String metadata)
    {
        if (m_mdcb)
        {
            String title, artist;

            auto i0 = metadata.lastIndexOf('.');
            if (i0 < 0) i0 = metadata.length();

            auto i1 = metadata.indexOf(" - ");
            if (i1 > 0)
            {
                title = metadata.substring(i1 + 3, i0);
                artist = metadata.substring(0, i1);
            }
            else
            {
                title = metadata.substring(0, i0);
            }

            if (!title.isEmpty())
                m_mdcb(audio_tools::MetaDataType::Title, title.c_str(), title.length());

            if (!artist.isEmpty())
                m_mdcb(audio_tools::MetaDataType::Artist, artist.c_str(), artist.length());
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    StorageAudioContext* StorageAudioContext::s_this = nullptr;

    void StorageAudioContext::s_initStreamCallback() 
    { 
        if (s_this) 
            s_this->initStreamCallback(); 
    }
    
    Stream* StorageAudioContext::s_nextStreamCallback(int offset)
    {
        if (s_this)
            return s_this->nextStreamCallback(offset);
        return nullptr;
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
        m_fileIndex = 0;
        m_dir = driver::storage.getFS().open(m_path);
        log_i("open dir: %s (%d)", m_dir.path(), int(m_dir));
    }

    Stream* StorageAudioContext::nextStreamCallback(int offset)
    {
        //audioPlayer.m_title.clear();
        //audioPlayer.m_artist.clear();

        m_file.close();
        m_fileIndex += offset;
        m_dir.rewindDirectory();
        for (int i = 0; i <= m_fileIndex; i++)
            m_file = m_dir.openNextFile();

        if (m_file)
        {
            fetchTitleAndAuthor(m_file.name());
            log_i("open file: %s (%d)", m_file.path(), m_fileIndex);
            return &m_file;
        }
        return nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////
}
