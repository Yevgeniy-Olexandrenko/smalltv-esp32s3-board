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
        float  getLatitude() const { return m_lat; }
        float  getLongitude() const { return m_lon; }
        int    getTZOffset() const;

        bool hasLocality() const { return !m_locality.isEmpty(); }
        const String& getLocality() const { return m_locality; }
        const String& getCountryCode() const { return m_countryCode; }
        const String& getCountryFlag() const { return m_countryFlag; }

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        void task() override;

        static void decodeCoords(float& lat, float& lon);
        static void encodeCoords(float& lat, float& lon);
        static void decodeTimeZone(int& tzh, int& tzm);
        static void encodeTimeZone(int& tzh, int& tzm);
        static void generateCountryFlag(const String& countryCode, String& countryFlag);

        void setTimeZone(int num);
        String getTimeZone() const;
        String getCoords() const;
        bool isNewCoords() const;

        bool requestGeoLocation();

    private:
        bool m_request;
        float m_lat;
        float m_lon;
        String m_locality;
        String m_countryCode;
        String m_countryFlag;
        details::GeoLocationRequests m_requests;
    };

    extern GeoLocation geoLocation;
}
