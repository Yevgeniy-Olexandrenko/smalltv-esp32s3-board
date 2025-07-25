#pragma once

#include <WebServer.h>
#include <FS.h>

namespace WebDAV
{
    class Handler;

    class Server
    {
    public:
        Server() : m_server(nullptr) {}
        Server(WebServer& server) : m_server(&server) {}
        virtual void setHandler(Handler& handler); 
        
        // helpers
        virtual String decodeURI(const String& encodedURI) const;
        virtual String encodeURI(const String& decodedURI) const;
        virtual String getContentType(const String& uri) const;
        virtual String getHttpDateTime(time_t timestamp) const;

        // get headers
        virtual bool hasHeader(const String& hdr) { return m_server->hasHeader(hdr); }
        virtual String getHeader(const String& hdr) { return m_server->header(hdr); }
        virtual size_t getContentLength() { return m_server->header("Content-Length").toInt(); }
        
        // send headers and content
        virtual void sendHeader(const String& hdr, const String& val) { m_server->sendHeader(hdr, val); }
        virtual void setContentLength(size_t length) { m_server->setContentLength(length); }
        virtual void sendContent(const String& content) { m_server->sendContent(content); }
        virtual void sendCode(int code, const String& contentType, const String& msg) { m_server->send(code, contentType, msg); }
        virtual void sendCode(int code, const String& msg) { m_server->send(code, "text/plain", msg); }

    private:
        WebServer* m_server;
    };

    class FileSystem
    {
    public:
        using QuotaSz = unsigned long long;
        using QuotaCb = std::function<void(fs::FS& fs, QuotaSz& available, QuotaSz& used)>;

        FileSystem(fs::FS& fs, const String& name, QuotaCb quotaCb)
            : m_fs(fs)
            , m_name(name)
            , m_quotaCb(quotaCb)
        {}

        fs::FS* operator->() { return &m_fs; }

        String resolveURI(fs::File& file);
        String resolvePath(const String& decodedURI);

        bool deleteRecursive(const String& path);

        const String& getName() const { return m_name; }
        bool getQuota(QuotaSz& available, QuotaSz& used);

    private:
        fs::FS& m_fs;
        String  m_name;
        QuotaCb m_quotaCb;
    };

    class Handler : public RequestHandler
    {
        class Resource
        {
        public:
            Resource(Handler& handler, const String& uri, time_t modified);
            Resource(Handler& handler, const String& uri, time_t modified, size_t size);

            void setQuota(FileSystem::QuotaSz available, FileSystem::QuotaSz used);
            String toString() const;

        private:
            String buildProp(const String& prop, const String& val) const;
            String buildOptProp(const String& prop, const String& val) const;

        private:
            String m_href, m_resourceType, m_lastModified;
            String m_contentLength, m_contentType, m_etag;
            String m_availableBytes, m_usedBytes; // RFC 4331
        };

    public:
        void begin(const Server& server);
        void addFS(fs::FS& fs, const String& mountName, FileSystem::QuotaCb quotaCb = nullptr);

    protected:
        bool canHandle(HTTPMethod method, String uri) override;
        bool handle(WebServer& server, HTTPMethod method, String uri) override;

    private:
        FileSystem* getMountedFS(const String& uri);
        String getETag(size_t size, time_t modified) const;

        void handleOPTIONS  ();
        bool handlePROPFIND (const String& decodedURI);
        void handleMKCOL    (FileSystem& wdfs, const String& path);
        void handleDELETE   (FileSystem& wdfs, const String& path);
        void handleGET      (FileSystem& wdfs, const String& path);
        void handlePUT      (FileSystem& wdfs, const String& path);
        void handleCOPY     (FileSystem& wdfs, const String& path, const String& dest);
        void handleMOVE     (FileSystem& wdfs, const String& path, const String& dest);

    private:
        Server m_server;
        std::vector<FileSystem> m_mountedFS;
    };
}
