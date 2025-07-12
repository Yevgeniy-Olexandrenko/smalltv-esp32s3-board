#include <HTTPClient.h>
#include "InternetAccess.h"
#include "services/WiFiConnection.h"

namespace service::details
{
    bool InternetAccess::available() const
    {
        return m_available;
    }

    void InternetAccess::update()
    {
        if (service::wifiConnection.isConnectedToAP())
        {
            if (m_timer.elapsed())
            {
                // restart timer to wait for the next check
                m_timer.start(1000 * CHECK_PERIOD_SEC);

                // try to check if internet is available
                HTTPClient http;
                http.setConnectTimeout(1000 * CHECK_TIMEOUT_SEC);
                http.begin("http://clients3.google.com/generate_204");
                m_available = (http.GET() == 204);
                http.end();
            }
        }
        else
        {
            // invalidate for instant check
            m_timer.stop();
            m_available = false;
        }
    }
}
