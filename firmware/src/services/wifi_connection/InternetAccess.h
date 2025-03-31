#pragma once

namespace service::wifi_connection
{
    class InternetAccess
    {
        constexpr static int PING_INTERVAL_MS = 20000;
        constexpr static int PING_TIMEOUT_MS  = 2000;

    public:
        InternetAccess();

        bool available() const;
        void update();

    private:
        void startPing();
        void checkPingResponse();
        void handlePingError();
        void stopPing();

    private:
        unsigned long m_pingStartTime;
        bool m_pingSuccessful;
        bool m_pending;
        int m_sock;
    };
}
