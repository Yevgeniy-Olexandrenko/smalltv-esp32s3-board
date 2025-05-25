#include "FromIPAddress.h"

namespace service::geo_location
{
    bool FromIPAddress::request(float& lat, float& lon, int& tzh, int& tzm)
    {
        const auto host = "ipapi.co";
        const auto port = 443;

        auto fail = [&]() { m_client.stop(); m_json.clear(); return false; };
        auto success = [&]() { m_client.stop(); m_json.clear(); return true; };

        m_client.setInsecure();
        if (m_client.connect(host, port)) 
        {
            // send HTTP request
            m_client.println("GET /json/ HTTP/1.1");
            m_client.print("Host: ");   
            m_client.println(host);
            m_client.println("User-Agent: esp-idf/1.0 esp32");
            m_client.println("Connection: close");
            if (m_client.println() == 0) 
                return fail();

            // check HTTP status and headers
            char status[32] = { 0 };
            m_client.readBytesUntil('\r', status, sizeof(status));
            if (strcmp(status + 9, "200 OK") != 0 || !m_client.find("\r\n\r\n")) 
                return fail();

            // parse JSON object  
            DeserializationError error = deserializeJson(m_json, m_client);
            if (!error)
            {
                lat = m_json["latitude"].as<float>();
                lon = m_json["longitude"].as<float>();
                auto off = m_json["utc_offset"].as<int>();
                log_i("off: %d", off);
                tzh = int8_t(off / 100);
                tzm = int8_t(off % 100);
                return success();
            }
        }  
        return fail();
    }
}
