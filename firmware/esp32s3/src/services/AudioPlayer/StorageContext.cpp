#ifndef NO_AUDIO

#include <random>
#include <AudioTools.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/CodecAACHelix.h>
#include "StorageContext.h"
#include "drivers/Storage.h"

namespace service::details
{
    StorageContext* StorageContext::s_this = nullptr;
    
    Stream* StorageContext::s_nextStreamCallback(int offset)
    {
        return (s_this ? s_this->nextStreamCallback(offset) : nullptr);
    }

    Stream* StorageContext::s_indexStreamCallback(int index)
    {
        return (s_this ? s_this->indexStreamCallback(index) : nullptr);
    }

    void StorageContext::fetchStorageExts(std::vector<String>& exts)
    {
        exts.clear();

        const auto basePath = String("/audio");
        const auto prefixSize = basePath.length() + 1;

        File file = driver::storage.getFS().open(basePath);
        if (!file.isDirectory()) return;

        while (true)
        {
            auto isDir = false;
            auto filePath = file.getNextFileName(&isDir);
            if (filePath.isEmpty()) break;

            if (isDir)
                exts.push_back(filePath.substring(prefixSize));
        }
    }

    void StorageContext::fetchStorageFilelistsForExt(const String &ext, std::vector<String> &filelists)
    {
        if (!ext.isEmpty())
        {
            auto strExt = ext;
            strExt.toLowerCase();

            const auto basePath = "/audio/" + ext;
            const auto prefixSize = basePath.length() + 1;

            std::function<bool(const String&)> hasFilesWithExt = 
            [&](const String& path)
            {
                bool isOK = false;
                File file = driver::storage.getFS().open(path);

                while (file)
                {
                    auto filePath = file.getNextFileName();
                    if (filePath.isEmpty())
                    {
                        isOK = false;
                        break;
                    }

                    auto i = filePath.lastIndexOf('/') + 1;
                    auto strName = filePath.substring(i);
                    strName.toLowerCase();

                    if (strName[0] != '.' && strName.endsWith('.' + strExt))
                    {
                        isOK = true;
                        break;
                    }
                }

                file.close();
                return isOK;
            };

            std::function<void(const String&, std::vector<String>&)> fetchFilelists =
            [&](const String& path, std::vector<String>& filelists) 
            {
                File file = driver::storage.getFS().open(path);
                if (!file.isDirectory()) return;

                std::vector<String> waitCheck;
                while (filelists.size() < filelists.capacity())
                {
                    auto isDir = false;
                    auto filePath = file.getNextFileName(&isDir);
                    if (filePath.isEmpty()) break;

                    if (isDir)
                    {
                        waitCheck.push_back(filePath);
                        if (hasFilesWithExt(filePath))
                            filelists.push_back(filePath.substring(prefixSize));
                    }
                }

                file.close();
                for (const auto& pathToCheck : waitCheck)
                    fetchFilelists(pathToCheck, filelists);
            };

            filelists.clear();
            filelists.reserve(256);

            fetchFilelists(basePath, filelists);
            filelists.shrink_to_fit();
        }
    }

    StorageContext::StorageContext(const String& ext, const String& dir, bool shuffle, bool loop)
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
                m_playlist.reserve(256);
                while (m_playlist.size() < m_playlist.capacity())
                {
                    auto filePath = file.getNextFileName();
                    if (filePath.isEmpty()) break;

                    auto i = filePath.lastIndexOf('/') + 1;
                    auto strName = filePath.substring(i);
                    strName.toLowerCase();

                    if (strName[0] != '.' && strName.endsWith('.' + strExt))
                    {
                        m_playlist.push_back(filePath.substring(i));
                    }
                }
                m_playlist.shrink_to_fit();

                if (shuffle)
                {
                    std::mt19937 rnd(millis());
                    std::shuffle(m_playlist.begin(), m_playlist.end(), rnd);
                }
                m_loop = loop;
            }
        }
    }

    StorageContext::~StorageContext()
    {
        end();
    }

    void StorageContext::begin()
    {
        if (!AudioContext::m_source && !AudioContext::m_decode)
        {
            // audio source
            auto source = new AudioSourceCallback();
            source->setCallbackNextStream(&s_nextStreamCallback);
            source->setCallbackSelectStream(&s_indexStreamCallback);
            m_source.reset(source);
            AudioContext::m_source = m_source.get();

            // decode chain
            m_decode.reset(new MP3DecoderHelix());
            m_filter.reset(new MetaDataFilterDecoder(*m_decode));
            AudioContext::m_decode = m_filter.get();
        }
    }

    void StorageContext::end()
    {
        AudioContext::m_source = nullptr;
        AudioContext::m_decode = nullptr;
        m_source.reset();
        m_decode.reset();
        m_filter.reset();
    }

    Stream* StorageContext::nextStreamCallback(int offset)
    {
        return (updatePlaylistIndex(offset) ? openPlaylistItemStream() : nullptr);
    }

    Stream *StorageContext::indexStreamCallback(int index)
    {
        return (setPlaylistIndex(index) ? openPlaylistItemStream() : nullptr);
    }

    Stream *StorageContext::openPlaylistItemStream()
    {
        auto path = m_path + "/" + m_playlist[m_index];
        m_file = driver::storage.getFS().open(path);

        if (m_file)
        {
            String name { m_file.name() };
            auto len = name.lastIndexOf('.');
            if (len < 0) len = name.length();
            m_newStreamCb(name.c_str(), len);

            log_i("[%d / %d] open file: %s", m_index, m_playlist.size(), m_file.path());
            return &m_file;
        }
        return nullptr;
    }
}
#endif
