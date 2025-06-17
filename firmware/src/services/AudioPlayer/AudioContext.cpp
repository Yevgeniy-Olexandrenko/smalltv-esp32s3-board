#ifndef NO_AUDIO

#include "AudioContext.h"

namespace service::details
{
    AudioContext::AudioContext() 
        : m_source(nullptr)
        , m_decode(nullptr)
        , m_newStreamCb(nullptr)
        , m_direction(1)
        , m_index(-1)
        , m_loop(false)
    {
    }

    bool AudioContext::updatePlayistIndex(int offset)
    {
        if (!m_playlist.empty())
        {
            const auto size = m_playlist.size();
            const auto next = m_index + offset;

            if (m_loop || (next >= 0 && next < size))
            {
                m_direction = offset;
                m_index += offset;

                if (m_index < 0) 
                    m_index += size;
                else 
                    m_index %= size;
                return true;
            }
        }
        return false;
    }

    bool AudioContext::setPlayistIndex(int index)
    {
        if (index >= 0 && index < m_playlist.size())
        {
            m_direction = 1;
            m_index = index;
            return true;
        }
        return false;
    }
}
#endif
