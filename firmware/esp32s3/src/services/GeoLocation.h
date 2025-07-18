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
        enum class Method { Manual, IPAddress, WiFiStations };

        constexpr static auto DEFAULT_METHOD     = int(Method::IPAddress);
        constexpr static auto DEFAULT_LATITUDE   = 50.4500f;    // Kyiv city
        constexpr static auto DEFAULT_LONGITUDE  = 30.5233f;    // coordinates
        constexpr static auto DEFAULT_TZ_OFFSET  = 200;         // +02:00 hours 
        constexpr static auto RETRY_PERIOD_SEC   = 5;           // 5 seconds

    public:
        void begin();
        void startRequest();

        float getLatitude() const { return m_lat; }
        float getLongitude() const { return m_lon; }
        int   getTZOffset() const;

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

        void setTimeZoneUI(int num);
        const String getTimeZoneUI() const;
        const String getCoordsUI() const;

        Method getMethod() const;
        bool hasNewCoords() const;
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
