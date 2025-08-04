#pragma once

#include <WebServer.h>
#include "WebDAVFS.h"

class WebDAVHandler;

class WebDAVServer
{
public:
    WebDAVServer() : m_server(nullptr) {}
    WebDAVServer(WebServer& server) : m_server(&server) {}
    virtual void setHandler(WebDAVHandler& handler); 
    WebServer& getWebServer() { return *m_server; }
    
    // helpers
    virtual String decodeURI(const String& encodedURI) const;
    virtual String encodeURI(const String& decodedURI) const;
    virtual String getContentType(const String& uri) const;
    virtual String getHttpDateTime(time_t timestamp) const;
    virtual String getETag(WebDAVFS::QuotaSz size, time_t modified) const;

    // get headers
    virtual bool hasHeader(const String& hdr) { return m_server->hasHeader(hdr); }
    virtual String getHeader(const String& hdr) { return m_server->header(hdr); }
    virtual size_t getContentLength() { return m_server->header("Content-Length").toInt(); }
    
    // send headers and content
    virtual void sendHeader(const String& hdr, const String& val) { m_server->sendHeader(hdr, val); }
    virtual void setContentLength(size_t length) { m_server->setContentLength(length); }
    virtual void sendContent(const String& content) { m_server->sendContent(content); }
    virtual void sendCode(int code, const String& contentType, const String& msg) 
    { 
        m_server->send(code, contentType, msg); 
    }
    virtual void sendCode(int code, const String& msg) 
    { 
        log_i("code: %d : %s", code, msg.c_str());
        m_server->send(code, "text/plain", msg); 
    }
    virtual void sendData(const uint8_t *buf, size_t size) { m_server->client().write(buf, size); }
    virtual void sendFile(fs::File file, const String& contentType) { m_server->streamFile(file, contentType); }

private:
    WebServer* m_server;
};

class WebDAVHandler : public RequestHandler
{
    class Resource
    {
    public:
        Resource(WebDAVServer& server, const String& uri, time_t modified);
        Resource(WebDAVServer& server, const String& uri, time_t modified, WebDAVFS::QuotaSz size);

        void setQuota(WebDAVFS::QuotaSz available, WebDAVFS::QuotaSz used);
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
    void begin(const WebDAVServer& server);
    void addFS(fs::FS& fs, const String& mountName, WebDAVFS::QuotaCb quotaCb = nullptr);

protected:
    bool canHandle(HTTPMethod method, String uri) override;
    bool canRaw(String uri) override;
    bool handle(WebServer& server, HTTPMethod method, String uri) override;
    void raw(WebServer& server, String uri, HTTPRaw& raw) override;

private:
    bool canHandleURI(const String& uri);
    WebDAVFS* resolveFS(const String& uri);

    void handleOPTIONS  ();
    bool handlePROPFIND (const String& decodedURI);
    void handleMKCOL    (WebDAVFS& fs, const String& path);
    void handleDELETE   (WebDAVFS& fs, const String& path);
    void handleGET      (WebDAVFS& fs, const String& path, bool isHEAD);
    void handlePUT_init (WebDAVFS& fs, const String& path);
    void handlePUT_loop (HTTPRaw& raw, fs::File& file, bool overwrite);
    void handleMOVE     (WebDAVFS& sfs, const String& spath, WebDAVFS& dfs, const String& dpath, bool overwrite);
    void handleCOPY     (WebDAVFS& sfs, const String& spath, WebDAVFS& dfs, const String& dpath, bool overwrite);

    bool sendErrorCode(int code, const String& msg);
    bool handleSrcDstCheck(WebDAVFS& sfs, const String& spath, WebDAVFS& dfs, const String& dpath);
    bool handleParentExists(WebDAVFS& fs, const String& path);
    bool handleOverwrite(WebDAVFS& fs, const String& path, bool overwrite, bool& overwritten);

private:
    WebDAVServer m_server;
    std::vector<WebDAVFS> m_mountedFS;
    fs::File m_uploadFile;
    bool m_uploadOverwrite;
};
