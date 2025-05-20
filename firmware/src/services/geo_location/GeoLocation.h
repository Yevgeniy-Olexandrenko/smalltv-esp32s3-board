#pragma once

#include <GeoIP.h>
#include "core/settings/Settings.h"

namespace service
{
    class GeoLocation : public settings::Provider
    {
    public:
        enum class Method { Manual, ByIPAddress, ByWiFiStations };

        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

        Method GetMethod() const { return m_method; }
        void SetMethod(Method method);

        float GetLatitude() const { return m_latitude; }
        float GetLongitude() const { return m_longitude; }

    private:
        float m_latitude =  50.4500f; // from -90.0 to +90.0
        float m_longitude = 30.5233f; // from -180.0 to +180.0

        Method m_method = Method::ByIPAddress;
    };

    extern GeoLocation geoLocation;
}
