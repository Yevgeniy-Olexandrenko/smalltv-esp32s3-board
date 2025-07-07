#pragma once

#include "core/tasks/Task.h"
#include "core/settings/Settings.h"
#include "GeoLocation/GeoLocationRequests.h"

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
        float  getLatitude() const;
        float  getLongitude() const;
        int    getTZOffset() const;

        const String& getLocality() const { return m_locality; }
        const String& getCountryCode() const { return m_countryCode; }
        const String& getCountryFlag() const { return m_countryFlag; }

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        void task() override;
        void encodeTimeZone(int& tzh, int& tzm) const;
        void decodeTimeZone(int& tzh, int& tzm) const;

        void setTimeZone(int num);
        String getTimeZone() const;
        String getCoordinates() const;

        bool requestGeoLocation();
        void generateCountryFlag();

    private:
        bool m_request;
        String m_locality;
        String m_countryCode;
        String m_countryFlag;
        details::GeoLocationRequests m_requests;
    };

    extern GeoLocation geoLocation;
}
