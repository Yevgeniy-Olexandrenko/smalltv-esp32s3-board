#pragma once

#include <WebServer.h>
#include "WebDAVFS.h"

class WebDAVHandler;

class WebDAVServer : public RequestHandler
{
public:
    WebDAVServer();
    WebDAVServer(WebServer& server);
    void attachHandler(WebDAVHandler& handler);
    
    // helpers
    String decodeURI(const String& encodedURI) const;
    String encodeURI(const String& decodedURI) const;
    String getContentType(const String& uri) const;
    String getHttpDateTime(time_t timestamp) const;
    String getETag(WebDAVFS::QuotaSz size, time_t modified) const;

    // headers
    bool   hasHeader(const String& hdr);
    String getHeader(const String& hdr);
    size_t getContentLength();
    void   setHeader(const String& hdr, const String& val);
    void   setContentLength(size_t length);

    // content
    void sendContent(const String& content);
    void sendCode(int code, const String& contentType, const String& msg);
    void sendCode(int code, const String& msg);
    void sendFile(fs::File file, const String& contentType);

protected:
    bool canHandle(HTTPMethod method, String uri) override;
    bool handle(WebServer& server, HTTPMethod method, String uri) override;
    bool canRaw(String uri) override;
    void raw(WebServer& server, String uri, HTTPRaw& raw) override;

private:
    WebServer* m_server;
    WebDAVHandler* m_handler;
};
