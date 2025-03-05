#include <random>
#include "AudioPlayList.h"
#include "drivers/storage/Storage.h"

namespace service_audio_player_impl
{
    void AudioPlayList::open(const String& resource, const String& ext)
    {
        if (m_list.empty() && !resource.isEmpty())
        {
            m_list.reserve(256);
            fs::File file = driver::storage.getFS().open(resource);
            if (file)
            {
                if (file.isDirectory())
                {
                    for (; m_list.size() < m_list.capacity();)
                    {
                        String path = file.getNextFileName();
                        if (path.isEmpty()) break;
                        insertIfValid(path, ext);
                    }
                }
                else
                {
                    insertIfValid(file.path(), ext);
                }
                file.close();
            }
        }
    }

    String AudioPlayList::getPrev()
    {
        String path = "";
        if (!m_list.empty())
        {
            path = m_list[m_index];
            if (m_index == 0) m_index = m_list.size();
            m_index--;
        }
        return path;
    }

    String AudioPlayList::getNext()
    {
        String path = "";
        if (!m_list.empty())
        {
            path = m_list[m_index++];
            if (m_index == m_list.size()) m_index = 0;
        }
        return path;
    }

    void AudioPlayList::close()
    {
        m_list.clear();
        m_index = 0;
    }

    void AudioPlayList::shuffle()
    {
        std::random_device randomDevice;
        std::mt19937 randomGenerator(randomDevice());
        std::shuffle(m_list.begin(), m_list.end(), randomGenerator);
    }

    void AudioPlayList::insertIfValid(const String& path, const String& ext)
    {
        // prepare strings for parsing
        auto strName = path.substring(path.lastIndexOf('/'));
        auto strExt  = ext;

        // extension and file name must not be empty
        if (strExt.isEmpty() || strName.isEmpty()) return;

        // ignore character case
        strName.toLowerCase();
        strExt.toLowerCase();

        // hidden files not allowed
        if (strName[0] != '.' && strName.endsWith('.' + strExt))
        {
            m_list.push_back(path);
        }
    }
}
