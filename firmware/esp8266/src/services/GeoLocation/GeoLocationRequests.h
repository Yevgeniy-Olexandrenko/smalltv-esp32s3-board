// #pragma once

// #include <HTTPClient.h>
// #include <ArduinoJson.h>

// namespace service::details
// {
//     class GeoLocationRequests
//     {
//     public:
//         bool requestUsingIPAddress(float& lat, float& lon, int& tzh, int& tzm);
//         bool requestUsingWiFiStations(float& lat, float& lon, int& tzh, int& tzm);
//         bool requestReverseGeocoding(float lat, float lon, String& locality, String& countryCode);

//     private:
//         bool requestGoogleGeolocationApi(float& lat, float& lon);
//         bool requestGoogleTimeZoneApi(float lat, float lon, long timestamp, int& tzh, int& tzm);

//     private:
//         HTTPClient m_http;
//         JsonDocument m_json;
//     };
// }
