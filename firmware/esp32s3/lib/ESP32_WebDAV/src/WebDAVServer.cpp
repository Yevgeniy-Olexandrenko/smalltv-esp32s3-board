#include <detail/mimetable.h>
#include "WebDAVHandler.h"
#include "WebDAVServer.h"

static const char* EXTRA_HEADERS[] = 
{
    "Depth", 
    "Destination", 
    "Overwrite", 
    "If-None-Match", 
    "If-Modified-Since",
    "Range"
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
}

void WebDAVServer::attachHandler(WebDAVHandler &handler)
{
    if (m_server)
    {
        if (!m_handler)
        {
            m_server->collectHeaders(EXTRA_HEADERS, sizeof(EXTRA_HEADERS) / sizeof(char*));
            m_server->addHandler(this);
        }
        m_handler = &handler;
    }
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
    String val = m_server->header(hdr);
    if (!val.isEmpty())
        log_i("%s : %s", hdr.c_str(), val.c_str());
    return val;
}

size_t WebDAVServer::getContentLength()
{
    return m_server->clientContentLength();
}

void WebDAVServer::setHeader(const String &hdr, const String &val)
{
    log_i("%s : %s", hdr.c_str(), val.c_str());
    m_server->sendHeader(hdr, val);
}

void WebDAVServer::setContentLength(size_t length)
{ 
    log_i("Content-Length : %d", length);
    m_server->setContentLength(length); 
}

void WebDAVServer::sendContent(const String& content)
{
    m_server->sendContent(content); 
}

void WebDAVServer::sendCode(int code, const String& contentType, const String& msg) 
{ 
    log_i("%d : %s", code, msg.c_str());
    m_server->send(code, contentType, msg); 
}

void WebDAVServer::sendCode(int code, const String& msg) 
{ 
    sendCode(code, "text/plain", msg);
}

void WebDAVServer::sendFile(fs::File file, const String& contentType, bool isHEAD)
{
    // special case for empty file
    size_t fileSize = file.size();
    if (fileSize == 0) 
    {
        setContentLength(0);
        sendCode(200, contentType, "");
        return;
    }

    // parse range header
    bool hasRange = false;
    size_t last = fileSize - 1;
    size_t from = 0, to = last;
    
    String hdrRange = getHeader("Range");
    if (hdrRange.startsWith("bytes=")) 
    {
        int sep = hdrRange.indexOf('-', 6);
        if (sep > 6) 
        {
            String strFrom = hdrRange.substring(6, sep);
            String strTo = hdrRange.substring(sep + 1);
            from = strFrom.isEmpty() ? 0 : strFrom.toInt();
            to = strTo.isEmpty() ? last : strTo.toInt();
            if (to > last) to = last;
            if (from > to || from > last) 
            {
                setHeader("Content-Range", "bytes */" + String(fileSize));
                sendCode(416, "Requested Range Not Satisfiable");
                return;
            }
            hasRange = true;
        }
    }

    // trying to seek the position in file
    size_t sendSize = to - from + 1;
    if (!file.seek(from)) 
    {
        sendCode(500, "Failed to seek file");
        return;
    }

    // set headers and send response code
    if (hasRange) 
    {
        setHeader("Content-Range", "bytes " + String(from) + "-" + String(to) + "/" + String(fileSize));
        setContentLength(sendSize);
        sendCode(206, contentType, "");
    } 
    else 
    {
        setContentLength(fileSize);
        sendCode(200, contentType, "");
    }
    if (isHEAD) return;

    // allocate buffer in PSRAM if possible
    log_i("begin file transfer");
    const size_t BUF_SIZE = 4096;
    uint8_t*  buf = (uint8_t*)ps_malloc(BUF_SIZE);
    if (!buf) buf = (uint8_t*)malloc(BUF_SIZE);
    if (!buf) 
    {
        sendCode(500, "Out of memory");
        return;
    }

    // sending file contents
    WiFiClient client = m_server->client();
    for (size_t remaining = sendSize; remaining > 0 && client.connected(); yield())
    {
        size_t chunk = (remaining > BUF_SIZE ? BUF_SIZE : remaining);
        int readBytes = file.read(buf, chunk);
        if (readBytes <= 0) break;
        size_t writeBytes = client.write(buf, readBytes);
        if (writeBytes != size_t(readBytes)) break;
        remaining -= readBytes;
    }
    client.flush();
    free(buf);
    log_i("end file transfer");
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
