#pragma once

#include <WebServer.h>

namespace fs { class FS; }

namespace core
{
    class WebDAV : public RequestHandler
    {
    public:
        void begin(WebServer& server, fs::FS& fs, const String& mountPoint);

    protected:
        bool canHandle(HTTPMethod method, String uri) override;
        bool handle(WebServer& server, HTTPMethod method, String uri) override;

    private:
        void handleOPTIONS(WebServer& server);
        void handleGET(WebServer& server);
        void handlePUT(WebServer& server);
        void handleDELETE(WebServer& server);
        void handleCOPY(WebServer& server);
        void handleMKCOL(WebServer& server);
        void handleMOVE(WebServer& server);
        void handlePROPFIND(WebServer& server);
        
    private:
        fs::FS* m_fs = nullptr;
        String  m_mp;
    };
}
