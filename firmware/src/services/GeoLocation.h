#pragma once

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "core/tasks/Task.h"
#include "core/settings/Settings.h"

namespace service
{
    class GeoLocation
        : public task::Task<8192, task::core::System, task::priority::Background>
        , public settings::Provider
    {
        constexpr static int RESTART_PERIOD_MS = 1000 * 60 * 60; // 1 hour
        constexpr static int RETRY_PERIOD_MS   = 1000 * 5;       // 5 seconds

    public:
        enum class Method { Manual, IPAddress, WiFiStations };

        void begin();
        void startRequest();

        Method getMethod() const;
        float getLatitude() const;
        float getLongitude() const;
        int getTZOffset() const;

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        void task() override;
        void encodeTimeZone(int& tzh, int& tzm) const;
        void decodeTimeZone(int& tzh, int& tzm) const;

        void setTimeZone(int num);
        String getTimeZone() const;

        bool requestGeoLocation();
        bool requestUsingIPAddress(float& lat, float& lon, int& tzh, int& tzm);
        bool requestUsingWiFiStations(float& lat, float& lon, int& tzh, int& tzm);
        bool requestGoogleGeolocationApi(float& lat, float& lon);
        bool requestGoogleTimeZoneApi(float lat, float lon, long timestamp, int& tzh, int& tzm);

    private:
        HTTPClient m_http;
        JsonDocument m_json;
    };

    extern GeoLocation geoLocation;
}
