#pragma once

#include "core/settings/Settings.h"

namespace service
{
    class GeoLocation : public settings::Provider
    {
        using Timestamp = unsigned long;

    public:
        enum class Method { Manual, IPAddress, WiFiStations };

        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

        Method getMethod() const;
        float  getLatitude() const;
        float  getLongitude() const;
        int    getTimeZoneOff() const;

    private:
        void encodeTimeZone(int& tzh, int& tzm) const;
        void decodeTimeZone(int& tzh, int& tzm) const;
        String getTimeZone() const;
        void setTimeZone(int num);

        void requestNewData();
        void confirmDataReceived();
        bool fetchNewData();

        bool fetchDataUsingIPAddress(float& lat, float& lon, int& tzh, int& tzm);
        bool fetchDataUsingWiFiStations(float& lat, float& lon, int& tzh, int& tzm);

        bool callGoogleGeolocationApi(float& lat, float& lon);
        bool callGoogleTimeZoneApi(float lat, float lon, long timestamp, int& tzh, int& tzm);

    private:
        Timestamp m_fetchTS;
    };

    extern GeoLocation geoLocation;
}
