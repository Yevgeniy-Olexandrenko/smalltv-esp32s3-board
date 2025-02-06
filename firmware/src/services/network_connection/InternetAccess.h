#pragma once

namespace service_network_connection_impl
{
    class InternetAccess
    {
        static const int PING_INTERVAL_MS = 20000;
        static const int PING_TIMEOUT_MS  = 2000;

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
