#include <Arduino.h>
#include <lwip/sockets.h>
#include <lwip/icmp.h>
#include <lwip/inet_chksum.h>
#include <lwip/ip.h>
#include "InternetAccess.h"

#ifdef IPADDR_NONE
#undef IPADDR_NONE
#endif

#ifdef INADDR_NONE
#undef INADDR_NONE
#endif

namespace service::wifi_connection
{
    // Returns a 16-bit Chip ID derived from the MAC address
    uint16_t getChipID()
    {
        uint64_t mac = ESP.getEfuseMac();
        return (uint16_t)(mac >> 32);
    }

    InternetAccess::InternetAccess()
        : m_pingSuccessful(false)
        , m_pingStartTime(0)
        , m_pending(false)
        , m_sock(-1)
    {}

    // Returns the last known status (true if the ping was successful).
    bool InternetAccess::available() const
    {
        return m_pingSuccessful;
    }

    // Call this regularly to process ping responses and start a new ping if needed.
    void InternetAccess::update()
    {
        unsigned long now = millis();
        if (m_pending)
        {
            // Check timeout or response
            if (now - m_pingStartTime >= PING_TIMEOUT_MS)
                handlePingError();
            else
                checkPingResponse();
        }
        else if (now - m_pingStartTime >= PING_INTERVAL_MS)
        {
            startPing();
        }
    }

    // Starts an asynchronous (non-blocking) ICMP ping to 8.8.8.8
    void InternetAccess::startPing()
    {
        m_pingStartTime = millis();
        m_sock = socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP);
        if (m_sock < 0)
        {
            handlePingError();
            return;
        }

        int flags = fcntl(m_sock, F_GETFL, 0);
        fcntl(m_sock, F_SETFL, flags | O_NONBLOCK);

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("8.8.8.8");

        struct icmp_echo_hdr echoReq;
        memset(&echoReq, 0, sizeof(echoReq));
        echoReq.type = ICMP_ECHO;
        echoReq.code = 0;
        echoReq.id = htons(getChipID());
        echoReq.seqno = htons(0);
        echoReq.chksum = 0;
        echoReq.chksum = inet_chksum(&echoReq, sizeof(echoReq));

        int sent = sendto(m_sock, &echoReq, sizeof(echoReq), 0, (struct sockaddr *)&addr, sizeof(addr));
        if (sent < 0)
        {
            handlePingError();
            return;
        }
        m_pending = true;
    }

    // Checks for a non-blocking ping response
    void InternetAccess::checkPingResponse()
    {
        char buf[64];
        int received = recv(m_sock, buf, sizeof(buf), 0);
        if (received > 0)
        {
            // Verify there's enough data for IP and ICMP headers
            struct ip_hdr *iphdr = (struct ip_hdr *)buf;
            int iphdr_len = IPH_HL(iphdr) * 4;

            if (received >= iphdr_len + (int)sizeof(struct icmp_echo_hdr))
            {
                struct icmp_echo_hdr *echoReply = (struct icmp_echo_hdr *)(buf + iphdr_len);
                m_pingSuccessful = (
                    echoReply->type == ICMP_ER &&
                    echoReply->id == htons(getChipID()) &&
                    echoReply->seqno == htons(0)
                );
            }
            else m_pingSuccessful = false;
            stopPing();
        }
    }

    // Handles repeated ping error steps
    void InternetAccess::handlePingError()
    {
        stopPing();
        m_pingSuccessful = false;
    }

    // Closes the socket and resets the ping state
    void InternetAccess::stopPing()
    {
        if (m_sock >= 0)
        {
            close(m_sock);
            m_sock = -1;
        }
        m_pending = false;
    }
}
