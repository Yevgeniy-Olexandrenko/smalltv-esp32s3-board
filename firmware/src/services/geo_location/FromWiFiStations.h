#pragma once

namespace service::geo_location
{
    class FromWiFiStations
    {
    public:
        bool request(float& lat, float& lon, int& tzh, int& tzm) { return false; }
    };
}
