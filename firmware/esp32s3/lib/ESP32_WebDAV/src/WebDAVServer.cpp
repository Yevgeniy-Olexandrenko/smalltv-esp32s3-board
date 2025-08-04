#include <detail/mimetable.h>
#include "WebDAVHandler.h"
#include "WebDAVServer.h"

static const char* EXTRA_HEADERS[] = 
{
    "Depth", 
    "Destination", 
    "Overwrite", 
    "If-None-Match", 
    "If-Modified-Since"
};

WebDAVServer::WebDAVServer()
    : m_server(nullptr)
    , m_handler(nullptr)
{
}

WebDAVServer::WebDAVServer(WebServer &server)
    : m_server(&server)
    , m_handler(nullptr)
{
    m_server->collectHeaders(EXTRA_HEADERS, sizeof(EXTRA_HEADERS) / sizeof(char*));
    m_server->addHandler(this);
}

void WebDAVServer::attachHandler(WebDAVHandler &handler)
{
    m_handler = &handler;
}

String WebDAVServer::decodeURI(const String &encodedURI) const
{
    return m_server->urlDecode(encodedURI);
}

String WebDAVServer::encodeURI(const String &decodedURI) const
{
    static const char hex[] = "0123456789ABCDEF";
    String encoded;
    encoded.reserve(decodedURI.length() * 3);
    for (size_t i = 0; i < decodedURI.length(); ++i) {
        char c = decodedURI[i];
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' ||
            c == '.' || c == '~' ||
            c == '/') 
        {
            encoded += c;
        } else {
            encoded += '%';
            encoded += hex[(c >> 4) & 0x0F];
            encoded += hex[c & 0x0F];
        }
    }
    return encoded;
}

String WebDAVServer::getContentType(const String& uri) const
{
    for (const auto& e : mime::mimeTable)
        if (uri.endsWith(e.endsWith)) return e.mimeType;
    return mime::mimeTable[mime::none].mimeType;
}

String WebDAVServer::getHttpDateTime(time_t timestamp) const
{
    char buf[40];
    struct tm* tmstruct = gmtime(&timestamp);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tmstruct);
    return String(buf);
}

String WebDAVServer::getETag(WebDAVFS::QuotaSz size, time_t modified) const
{
    return ('"' + String(size, 16) + '-' + String(modified, 16) + '"');
}

bool WebDAVServer::hasHeader(const String &hdr)
{
    return m_server->hasHeader(hdr);
}

String WebDAVServer::getHeader(const String &hdr)
{
    return m_server->header(hdr);
}

size_t WebDAVServer::getContentLength()
{
    return m_server->clientContentLength();
}

void WebDAVServer::setHeader(const String &hdr, const String &val)
{
    m_server->sendHeader(hdr, val);
}

void WebDAVServer::setContentLength(size_t length)
{ 
    m_server->setContentLength(length); 
}

void WebDAVServer::sendContent(const String& content)
{
    m_server->sendContent(content); 
}

void WebDAVServer::sendCode(int code, const String& contentType, const String& msg) 
{ 
    log_i("code: %d : %s", code, msg.c_str());
    m_server->send(code, contentType, msg); 
}

void WebDAVServer::sendCode(int code, const String& msg) 
{ 
    sendCode(code, "text/plain", msg);
}

void WebDAVServer::sendFile(fs::File file, const String& contentType)
{
    m_server->streamFile(file, contentType); 
}

bool WebDAVServer::canHandle(HTTPMethod method, String uri)
{
    return (m_handler ? m_handler->canHandle(method, uri) : false);
}

bool WebDAVServer::handle(WebServer &server, HTTPMethod method, String uri)
{
    return (m_handler ? m_handler->handle(method, uri) : false);
}

bool WebDAVServer::canRaw(String uri)
{
    return (m_handler ? m_handler->canRaw(m_server->method(), uri) : false);
}

void WebDAVServer::raw(WebServer &server, String uri, HTTPRaw &raw)
{
    if (m_handler) m_handler->raw(server.method(), uri, raw);
}
