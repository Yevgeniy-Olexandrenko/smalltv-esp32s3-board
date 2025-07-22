#pragma once

#include <WebServer.h>
#include <FS.h>

namespace core
{
    class WebDAV : public RequestHandler
    {
        enum class Resource { None, File, Dir };
        enum class Depth { None, Child, All };

        struct MountedFS { fs::FS* fs; String mp, alias; };

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
        void handleOPTIONS();
        void handleGET();
        void handlePUT();
        void handleDELETE();
        void handleCOPY();
        void handleMKCOL();
        void handleMOVE();
        void handlePROPFIND();

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

        String getFSPath();
        Resource getResource(const String& fsPath);
        Depth getDepth();
        

    private:
        time_t m_start;
        WebServer* m_server = nullptr;
        std::vector<MountedFS> m_mounted;
        int m_mpIndex;
    };
}
