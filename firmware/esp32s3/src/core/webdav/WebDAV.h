#pragma once

#include <WebServer.h>
#include <FS.h>

namespace core
{
    class WebDAVFileSystem
    {
    public:
        using QuotaSz = unsigned long long;
        using QuotaCb = std::function<void(fs::FS& fs, QuotaSz& available, QuotaSz& used)>;

        WebDAVFileSystem(fs::FS& fs, const String& name, QuotaCb quotaCb)
            : m_fs(fs)
            , m_name(name)
            , m_quotaCb(quotaCb)
        {}

        fs::FS* operator->() { return &m_fs; }

        String resolveURI(fs::File& file);
        String resolvePath(const String& decodedURI);

        const String& getName() const { return m_name; }
        bool getQuota(QuotaSz& available, QuotaSz& used);

    private:
        fs::FS& m_fs;
        String  m_name;
        QuotaCb m_quotaCb;
    };

    class WebDAVHandler : public RequestHandler
    {
        class ResourceProps
        {
        public:
            ResourceProps(const String& uri, time_t modified);
            ResourceProps(const String& uri, time_t modified, size_t size);

            void setQuota(unsigned long available, unsigned long used);
            String toString() const;

        private:
            String m_href, m_resourceType, m_lastModified;
            String m_contentLength, m_contentType, m_etag;
            String m_availableBytes, m_usedBytes; // RFC 4331
        };

    public:
        void begin(WebServer& server);
        void addFS(fs::FS& fs, const String& mountName, WebDAVFileSystem::QuotaCb quotaCb = nullptr);

    protected:
        bool canHandle(HTTPMethod method, String uri) override;
        bool handle(WebServer& server, HTTPMethod method, String uri) override;

    private:
        void handleOPTIONS  ();
        bool handlePROPFIND (const String& decodedURI);
        void handleMKCOL    (WebDAVFileSystem& wdfs, const String& path);
        void handleDELETE   (WebDAVFileSystem& wdfs, const String& path);
        void handleGET      (WebDAVFileSystem& wdfs, const String& path);
        void handlePUT      (WebDAVFileSystem& wdfs, const String& path);
        void handleCOPY     (WebDAVFileSystem& wdfs, const String& path, const String& dest);
        void handleMOVE     (WebDAVFileSystem& wdfs, const String& path, const String& dest);
        
        /////////////////////////////////

        static String decodeURI(const String& encoded);
        static String encodeURI(const String& decoded);
        static String getDateString(time_t date);
        static String getContentType(const String& uri);
        static String getETag(const String& uri, time_t modified);

        static String buildProp(const String &prop, const String &val);
        static String buildOptProp(const String &prop, const String &val);

        /////////////////////////////////

        WebDAVFileSystem* getMountedFS(const String &uri);
        
    private:
        WebServer* m_server = nullptr;
        std::vector<WebDAVFileSystem> m_mountedFS;
    };
}
