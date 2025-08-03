#pragma once

#include <WebServer.h>
#include "FileSystem.h"

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
        virtual void sendData(const uint8_t *buf, size_t size) { m_server->client().write(buf, size); }
        virtual void sendFile(fs::File file, const String& contentType) { m_server->streamFile(file, contentType); }

    private:
        WebServer* m_server;
    };

    class Handler : public RequestHandler
    {
        class Resource
        {
        public:
            Resource(Handler& handler, const String& uri, time_t modified);
            Resource(Handler& handler, const String& uri, time_t modified, FileSystem::QuotaSz size);

            void setQuota(FileSystem::QuotaSz available, FileSystem::QuotaSz used);
            String toString() const;

        private:
            String reqProp(const String& prop, const String& val) const;
            String optProp(const String& prop, const String& val) const;

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
        FileSystem* resolveFS(const String& uri);
        String getETag(FileSystem::QuotaSz size, time_t modified) const;

        void handleOPTIONS  ();
        bool handlePROPFIND (const String& decodedURI);
        void handleMKCOL    (FileSystem& fs, const String& path);
        void handleDELETE   (FileSystem& fs, const String& path);
        void handleGET_HEAD (FileSystem& fs, const String& path, bool isHEAD);
        void handlePUT      (FileSystem& fs, const String& path);
        void handleMOVE     (FileSystem& sfs, const String& spath, FileSystem& dfs, const String& dpath, bool overwrite);
        void handleCOPY     (FileSystem& sfs, const String& spath, FileSystem& dfs, const String& dpath, bool overwrite);

    private:
        Server m_server;
        std::vector<FileSystem> m_mountedFS;
    };
}
