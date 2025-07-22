#pragma once

#include <WebServer.h>
#include <FS.h>

namespace core
{
    class WebDAV : public RequestHandler
    {
        enum class Resource { None, File, Dir };
        enum class Depth { None, Child, All };

    public:
        void begin(WebServer& server, fs::FS& fs, const String& mountPoint);

    protected:
        bool canHandle(HTTPMethod method, String uri) override;
        bool handle(WebServer& server, HTTPMethod method, String uri) override;

    private:
        void handleOPTIONS();
        void handleGET();
        void handlePUT();
        void handleDELETE();
        void handleCOPY();
        void handleMKCOL();
        void handleMOVE();
        void handlePROPFIND();

        String getFSPath();
        Resource getResource(const String& fsPath);
        Depth getDepth();

        void sendPropResponse(fs::File &curFile);
        void sendPropContent(const String& prop, const String& content);

        String toString(time_t date);
        String getETag(const String& uri, time_t lastWrite);
        String getContentType(const String& uri);

    private:
        WebServer* m_ws = nullptr;
        fs::FS* m_fs = nullptr;
        String m_mp;
    };
}
