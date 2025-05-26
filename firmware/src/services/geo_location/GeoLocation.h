#pragma once

#include "core/settings/Settings.h"
#include "FromIPAddress.h"
#include "FromWiFiStations.h"

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
        void requestNewData();
        void confirmDataReceived();
        bool hasNewData();

        String getTimeZone() const;

    private:
        Timestamp m_fetchTS;
        geo_location::FromIPAddress m_fromIPAddress;
        geo_location::FromWiFiStations m_fromWiFiStations;
    };

    extern GeoLocation geoLocation;
}
