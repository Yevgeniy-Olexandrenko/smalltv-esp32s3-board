#pragma once

#include <Arduino.h>
#include <time.h>
#include <sys/time.h>

namespace service
{
    class DateTime
    {
    public:
        void begin();
        bool isSynced() const;

        void setUTC(time_t timestamp);
        void setNow(time_t timestamp);
        time_t getNow() const;

        String timeToString() const;
        String dateToString() const;
        String timeToString(time_t timestamp) const;
        String dateToString(time_t timestamp) const;
    };

    extern DateTime dateTime;
}
