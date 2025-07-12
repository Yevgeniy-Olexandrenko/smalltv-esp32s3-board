#pragma once

#include <Arduino.h>

namespace core
{
    class Timer
    {
        time_t m_elapsedTS = 0;

    public:
        void start(time_t duration) { m_elapsedTS = millis() + duration; }
        void stop() { start(0); }

        bool elapsed() const { return millis() >= m_elapsedTS; }
        time_t remaining() const { return elapsed() ? 0 : m_elapsedTS - millis(); }    
    };
}