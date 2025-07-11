#include "DateTime.h"
#include "GeoLocation.h"

namespace service
{
    void DateTime::begin()
    {
        setNow(0);
        configTime(0, 0, "pool.ntp.org", "time.google.com");
    }

    void DateTime::setUTC(time_t timestamp)
    {
        struct timeval tv{ .tv_sec = timestamp, .tv_usec = 0 };
        settimeofday(&tv, nullptr);
    }

    void DateTime::setNow(time_t timestamp)
    {
        time_t off = service::geoLocation.getTZOffset();
        time_t now = timestamp > off ? timestamp - off : 0;
        setUTC(now);
    }

    time_t DateTime::getNow() const
    {
        time_t off = service::geoLocation.getTZOffset();
        time_t now = time(nullptr);
        return (now + off);
    }

    bool DateTime::isSynced() const
    {
        time_t now = time(nullptr);
        return (now > 100000);
    }

    String DateTime::timeToString() const
    {
        return timeToString(getNow());
    }

    String DateTime::dateToString() const
    {
        return dateToString(getNow());
    }

    String DateTime::timeToString(time_t timestamp) const
    {
        struct tm tm{};
        gmtime_r(&timestamp, &tm);
        char buf[9]; // "HH:MM:SS" + '\0'
        snprintf(buf, sizeof(buf),
            "%02d:%02d:%02d",
            tm.tm_hour, tm.tm_min, tm.tm_sec);
        return String(buf);
    }

    String DateTime::dateToString(time_t timestamp) const
    {
        struct tm tm{};
        gmtime_r(&timestamp, &tm);
        char buf[11]; // "DD.MM.YYYY" + '\0'
        snprintf(buf, sizeof(buf),
            "%02d.%02d.%04d",
            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
        return String(buf);
    }

    DateTime dateTime;
}
