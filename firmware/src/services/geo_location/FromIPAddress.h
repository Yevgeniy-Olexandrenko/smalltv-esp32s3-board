#pragma once

#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

namespace service::geo_location
{
    class FromIPAddress
    {
    public:
        bool request(float& lat, float& lon, int& tzh, int& tzm);

    private:
        WiFiClientSecure m_client;
        JsonDocument m_json;
    };
}
