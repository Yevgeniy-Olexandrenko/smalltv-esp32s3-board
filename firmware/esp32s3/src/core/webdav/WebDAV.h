#pragma once

#include <WebServer.h>
#include <FS.h>

namespace core
{
    class WebDAV : public RequestHandler
    {
        enum class Resource { None, File, Dir };
        enum class Depth { None, Child, All };

        struct FS { fs::FS* underlying; String mountPoint, alias; };

        class Props
        {
        public:
            Props(const String& uri, time_t modified, const String& name);
            Props(const String& uri, time_t modified, size_t size);
            String toString() const;

        private:
            String m_href, m_resourceType, m_lastModified;
            String m_displayName, m_contentLength, m_contentType, m_etag;
        };

    public:
        //void begin(WebServer& server, fs::FS& fs, const String& mountPoint);
        void begin(WebServer& server);
        void addFS(fs::FS& fs, const String& mountPoint, const String& alias = "");

    protected:
        bool canHandle(HTTPMethod method, String uri) override;
        bool handle(WebServer& server, HTTPMethod method, String uri) override;

    private:
        bool handleOPTIONS();
        bool handleGET();
        bool handlePUT();
        bool handleDELETE();
        bool handleCOPY();
        bool handleMKCOL();
        bool handleMOVE();
        bool handlePROPFIND();

        /////////////////////////////////

        static String decodeURI(const String& encoded);
        static String encodeURI(const String& decoded);
        static String getDateString(time_t date);
        static String getContentType(const String& uri);
        static String getETag(const String& uri, time_t modified);

        static String buildProp(const String &prop, const String &val);
        static String buildOptProp(const String &prop, const String &val);

        static String getFileURI(const String& mountPoint, fs::File& file);
        static String getFilePath(const String &mountPoint, const String &uri);

        /////////////////////////////////

        FS* getMountedFS(const String &uri);
        Resource getResource(FS* fs, const String& path);
        Depth getDepth();
        

    private:
        WebServer* m_server = nullptr;
        std::vector<FS> m_mounted;
    };
}
