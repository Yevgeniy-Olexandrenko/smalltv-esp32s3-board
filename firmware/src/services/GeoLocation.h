#pragma once

#include "core/tasks/Task.h"
#include "core/settings/Settings.h"

namespace service
{
    class GeoLocation
        : public task::Task<4096 * 4, task::core::System, task::priority::Background>
        , public settings::Provider
    {

    public:
        enum class Method { Manual, IPAddress, WiFiStations };

        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

        Method getMethod() const;
        float getLatitude() const;
        float getLongitude() const;
        int getTZOffset() const;

    private:
        void startRequest();
        void task() override;

        String getTimeZone() const;
        void setTimeZone(int num);

        void encodeTimeZone(int& tzh, int& tzm) const;
        void decodeTimeZone(int& tzh, int& tzm) const;

        bool requestGeoLocation();
        bool requestUsingIPAddress(float& lat, float& lon, int& tzh, int& tzm);
        bool requestUsingWiFiStations(float& lat, float& lon, int& tzh, int& tzm);
        bool requestGoogleGeolocationApi(float& lat, float& lon);
        bool requestGoogleTimeZoneApi(float lat, float lon, long timestamp, int& tzh, int& tzm);
    };

    extern GeoLocation geoLocation;
}
