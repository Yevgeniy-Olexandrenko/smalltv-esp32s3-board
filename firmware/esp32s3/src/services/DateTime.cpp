#include <esp_sntp.h>
#include <HTTPClient.h>
#include "DateTime.h"
#include "GeoLocation.h"
#include "WiFiConnection.h"

namespace service
{
    void DateTime::begin()
    {
        setNow(0);
        configTime(0, 0, "pool.ntp.org", "time.google.com");
        m_isSynced = false;
    }

    void DateTime::setUTC(time_t timestamp)
    {
        struct timeval tv{ .tv_sec = timestamp, .tv_usec = 0 };
        settimeofday(&tv, nullptr);
        m_isSynced = true;
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
        auto isSntpSynced = 
            (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED);
        return (m_isSynced || isSntpSynced);
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

    void service::DateTime::onConnectedToWiFi()
    {
        time_t timestamp;
        if (fetchGatewayTime(timestamp)) setNow(timestamp);
    }

    bool service::DateTime::fetchGatewayTime(time_t& timestamp)
    {
        if (service::wifiConnection.isConnectedToAP())
        {
            HTTPClient http;
            http.begin("http://" + WiFi.gatewayIP().toString() + "/");

            if (http.sendRequest("HEAD") > 0)
            {
                String date = http.header("Date");
                log_i("gateway date: %s", date.c_str());

                if (!date.isEmpty())
                {
                    char wkday[4], mon[4], tz[4];
                    int day, year, hh, mm, ss;
                    int parsed = sscanf(date.c_str(),
                        "%3s, %d %3s %d %d:%d:%d %3s",
                        wkday, &day, mon, &year, &hh, &mm, &ss, tz);

                    if (parsed == 8)
                    {
                        int month = monthFromAbbrev(mon);
                        if (month > 0 && month < 13)
                        {
                            int64_t days = daysFromCivil(year, month, day);
                            int64_t seconds = days * 86400LL + 
                                int64_t(hh * 3600LL) + 
                                int64_t(mm * 60LL) + 
                                int64_t(ss);

                            timestamp = (time_t)seconds;
                            return true;
                        }
                    }
                }
            }
            http.end();
        }
        return false;
    }

    // algorithm from Howard Hinnant (days_from_civil)
    int64_t service::DateTime::daysFromCivil(int y, unsigned m, unsigned d)
    {
        y -= (m <= 2);
        const int era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);
        const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        return era * 146097 + static_cast<int>(doe) - 719468;
    }

    int service::DateTime::monthFromAbbrev(const char* mon)
    {
        static const char* names[] = 
        {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
        for (int i = 0; i < 12; ++i)
            if (strncmp(mon, names[i], 3) == 0) return i + 1;
        return 0;
    }

    DateTime dateTime;
}
