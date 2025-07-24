#include "WebDAV.h"
#include "core/Strings.h"
#include <detail/mimetable.h>

#include <rom/miniz.h>
#define crc32(a, len) mz_crc32( 0xffffffff,(const unsigned char *)a, len)

namespace core
{
    void WebDAVHandler::begin(WebServer &server)
    {
        static const char* hdrs[] = 
        {
            "Depth", "Destination", "Overwrite",
            "If-None-Match", "If-Modified-Since", "Range"
        };

        m_server = &server;
        m_server->collectHeaders(hdrs, sizeof(hdrs) / sizeof(char*));
        m_server->addHandler(this);
        m_mountedFS.clear();
    }

    void WebDAVHandler::addFS(fs::FS& fs, const String& mountName, WebDAVFileSystem::QuotaCb quotaCb)
    {
        if (!mountName.isEmpty())
        {
            if (mountName.startsWith("/"))
                m_mountedFS.emplace_back(fs, mountName, quotaCb);
            else
                m_mountedFS.emplace_back(fs, "/" + mountName, quotaCb);
        }
    }

    bool WebDAVHandler::canHandle(HTTPMethod method, String uri)
    {
        switch(method)
        {
            case HTTP_OPTIONS:
            case HTTP_PROPFIND:
                return true;

            case HTTP_MKCOL:
            case HTTP_DELETE:
            case HTTP_GET:
            case HTTP_PUT:
            case HTTP_COPY:
            case HTTP_MOVE:
                String decodedURI = decodeURI(uri);
                WebDAVFileSystem* wdfs = getMountedFS(decodedURI);
                return (wdfs != nullptr);
        }
        return false;
    }

    bool WebDAVHandler::handle(WebServer& server, HTTPMethod method, String uri)
    {
        if (method == HTTP_OPTIONS)
        {
            handleOPTIONS();
            return true;
        }
        
        String decodedURI = decodeURI(uri);
        if (method == HTTP_PROPFIND)
        {
            return handlePROPFIND(decodedURI);
        }

        WebDAVFileSystem* wdfs = getMountedFS(decodedURI);
        if (wdfs)
        {
            String path = wdfs->resolvePath(decodedURI);
            String destination = server.header("Destination");
            switch(method)
            {
                case HTTP_MKCOL:  handleMKCOL  (*wdfs, path); break;
                case HTTP_DELETE: handleDELETE (*wdfs, path); break;
                case HTTP_GET:    handleGET    (*wdfs, path); break;
                case HTTP_PUT:    handlePUT    (*wdfs, path); break;
                case HTTP_COPY:   handleCOPY   (*wdfs, path, destination); break;
                case HTTP_MOVE:   handleMOVE   (*wdfs, path, destination); break;
            }
            return true;
        }        
        return false;
    }

    // done
    void WebDAVHandler::handleOPTIONS()
    {
        log_i("handleOPTIONS");
        m_server->sendHeader("DAV", "1");
        m_server->sendHeader("Allow", "OPTIONS, GET, PROPFIND, PUT, DELETE, MKCOL, COPY, MOVE");
        m_server->send(200);
    }

    // done
    bool WebDAVHandler::handlePROPFIND(const String& decodedURI)
    {
        bool depthChild = (m_server->header("Depth") == "1");
        std::vector<ResourceProps> resources;
        if (decodedURI == "/")
        {
            log_i("handlePROPFIND (root)");

            // collect the list of mounted file systems
            resources.emplace_back(decodedURI, time(nullptr));
            if (depthChild)
            {
                for (auto& wdfs : m_mountedFS)
                {
                    if (fs::File childFile = wdfs->open("/"))
                    {
                        resources.emplace_back(
                            wdfs.resolveURI(childFile), childFile.getLastWrite());
                        childFile.close();

                        WebDAVFileSystem::QuotaSz available, used;
                        if (wdfs.getQuota(available, used))
                            resources.back().setQuota(available, used);
                    }
                }
            }
        }
        else
        {
            // check if request can be processed
            WebDAVFileSystem* wdfs = getMountedFS(decodedURI);
            if (!wdfs) return false;

            log_i("handlePROPFIND (resources)");

            // check if resource available
            String path = wdfs->resolvePath(decodedURI);
            if (!(*wdfs)->exists(path))
            {
                m_server->send(404);
                return true;
            }

            // collect the list of resources
            fs::File baseFile = (*wdfs)->open(path);
            resources.emplace_back(
                wdfs->resolveURI(baseFile), baseFile.getLastWrite(), baseFile.size());
            if (baseFile.isDirectory() && depthChild)
            {
                while (fs::File childFile = baseFile.openNextFile())
                {
                    resources.emplace_back(
                        wdfs->resolveURI(childFile), childFile.getLastWrite(), childFile.size());
                    childFile.close();
                }
            }
            baseFile.close();
        }

        // compute content length
        const String xmlPreamble = 
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
            "<D:multistatus xmlns:D=\"DAV:\">";
        const String xmlEpilogue =
            "</D:multistatus>";
        size_t contentLength = 0;
        for (const auto& resource : resources)
        {
            contentLength += resource.toString().length();
        }
        contentLength += xmlPreamble.length();
        contentLength += xmlEpilogue.length();

        // send status and content
        m_server->setContentLength(contentLength);
        m_server->send(207, "text/xml; charset=\"utf-8\"", "");
        m_server->sendContent(xmlPreamble);
        for (const auto& resource : resources)
        {
            m_server->sendContent(resource.toString());
        }
        m_server->sendContent(xmlEpilogue);
        return true;
    }

    void WebDAVHandler::handleMKCOL(WebDAVFileSystem& wdfs, const String& path)
    {   
        log_i("handleMKCOL");

        // check that the request should not have a body
        if (m_server->hasHeader("Content-Length"))
        {
            if (m_server->header("Content-Length").toInt() > 0)
                return m_server->send(415, "text/plain", "MKCOL with body not supported");
        }

        // check if resource is NOT available
        if (wdfs->exists(path)) 
            return m_server->send(405, "text/plain", "File or directory already exists");

        // check if parent directory exists
        int slashIdx = path.lastIndexOf('/');
        String parent = (slashIdx > 0) ? path.substring(0, slashIdx) : "/";
        if (!wdfs->exists(parent)) 
            return m_server->send(409, "text/plain", "Parent directory does not exist");

        // trying to create the directory
        if (!wdfs->mkdir(path)) 
            return m_server->send(500, "text/plain", "Failed to create directory");
        m_server->send(201, "text/plain", "Directory created");
    }

    void WebDAVHandler::handleDELETE(WebDAVFileSystem& wdfs, const String& path)
    { 
        log_i("handleDELETE");
        m_server->send(501);
    }

    void WebDAVHandler::handleGET(WebDAVFileSystem& wdfs, const String& path)
    {
        log_i("handleGET");
        m_server->send(501);
    }

    void WebDAVHandler::handlePUT(WebDAVFileSystem& wdfs, const String& path)    
    { 
        log_i("handlePUT"); 
        m_server->send(501);
    }

    void WebDAVHandler::handleCOPY(WebDAVFileSystem& wdfs, const String& path, const String& dest) 
    {
        log_i("handleCOPY");
        m_server->send(501);
    }

    void WebDAVHandler::handleMOVE(WebDAVFileSystem& wdfs, const String& path, const String& dest)
    { 
        log_i("handleMOVE");
        m_server->send(501);
    }

    ////////////////////////////////////////////////////////////////////////////

    String WebDAVHandler::decodeURI(const String& encoded)
    {
        return WebServer::urlDecode(encoded);
    }

    // TODO: check implementation!
    String WebDAVHandler::encodeURI(const String& decoded)
    {
        static const char hex[] = "0123456789ABCDEF";
        String encoded;
        encoded.reserve(decoded.length() * 3);
        for (size_t i = 0; i < decoded.length(); ++i) {
            char c = decoded[i];
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

    String WebDAVHandler::getDateString(time_t date)
    {
        const char *months[] = 
        {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

        const char *wdays[] = 
        {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
        };

        // get & convert time to required format
        // Tue, 13 Oct 2015 17:07:35 GMT
        tm* gTm = gmtime(&date);
        char buf[40];
        snprintf(buf, sizeof(buf), "%s, %02d %s %04d %02d:%02d:%02d GMT",
            wdays[gTm->tm_wday],
            gTm->tm_mday,
            months[gTm->tm_mon],
            gTm->tm_year + 1900,
            gTm->tm_hour,
            gTm->tm_min,
            gTm->tm_sec);
        return buf;
    }

    String WebDAVHandler::getContentType(const String& uri)
    {
        for (const auto& e : mime::mimeTable)
            if (uri.endsWith(e.endsWith)) return e.mimeType;
        return mime::mimeTable[mime::none].mimeType;
    }

    String WebDAVHandler::getETag(const String &uri, time_t modified)
    {
        char buf[uri.length() + 32];
        sprintf(buf, "%s%lu", uri.c_str(), (unsigned long)modified);
        uint32_t crc = crc32(buf, strlen(buf));
        sprintf(buf, "\"%08x\"", crc);
        return buf;
    }

    ////////////////////////////////////////////////////////////////////////////

    String WebDAVHandler::buildProp(const String &prop, const String &val)
    {
        // examples:
        // <D:resourcetype/>
        // <D:status>HTTP/1.1 200 OK</D:status>
        String str;
        if (val.isEmpty())
        {
            str.reserve(prop.length() + 5);
            str = "<D:" + prop + "/>";
        }
        else
        {
            str.reserve(2 * prop.length() + 9 + val.length());
            str = "<D:" + prop + ">" + val + "</D:" + prop + ">";
        }
        return str;
    }

    String WebDAVHandler::buildOptProp(const String &prop, const String &val)
    {
        if (val.isEmpty()) return "";
        return buildProp(prop, val);
    }

    ////////////////////////////////////////////////////////////////////////////

    WebDAVFileSystem* WebDAVHandler::getMountedFS(const String &uri)
    {
        for (auto& wdfs : m_mountedFS)
            if (uri.startsWith(wdfs.getName())) return &wdfs;
        return nullptr;
    }

    ///////////////////////////////////////////////////////

    WebDAVHandler::ResourceProps::ResourceProps(const String &uri, time_t modified)
        : m_href(encodeURI(uri))
        , m_lastModified(getDateString(modified))
        , m_etag(getETag(uri, modified))
    {
        if (uri.endsWith("/"))
            m_resourceType = "<D:collection/>";
    }

    WebDAVHandler::ResourceProps::ResourceProps(const String &uri, time_t modified, size_t size)
        : m_href(encodeURI(uri))
        , m_lastModified(getDateString(modified))
        , m_etag(getETag(uri, modified))
    {
        if (uri.endsWith("/"))
            m_resourceType = "<D:collection/>";
        else
        {
            m_contentLength = String(size);
            m_contentType = getContentType(uri);
        }
    }

    void WebDAVHandler::ResourceProps::setQuota(unsigned long available, unsigned long used)
    {
        m_availableBytes = String(available);
        m_usedBytes = String(used);
    }

    String WebDAVHandler::ResourceProps::toString() const
    {
        return
        buildProp("response",
            buildProp("href", m_href) +
            buildProp("propstat",
                buildProp("prop",
                    buildProp   ("resourcetype", m_resourceType) +
                    buildOptProp("getcontentlength", m_contentLength) +
                    buildOptProp("getcontenttype", m_contentType) +
                    buildOptProp("getetag", m_etag) +
                    buildProp   ("getlastmodified", m_lastModified) +
                    buildOptProp("quota-available-bytes", m_availableBytes) +
                    buildOptProp("quota-used-bytes", m_usedBytes)
                ) +
                buildProp("status", "HTTP/1.1 200 OK")
            )
        );
    }

    ////////////////////////////////////////////////////////////////////////////

    String WebDAVFileSystem::resolveURI(fs::File &file)
    {
        String path = file.path();
        if (!path.startsWith("/")) path = "/" + path;
        if (file.isDirectory() && !path.endsWith("/")) path += "/";
        return (m_name + path);
    }

    String WebDAVFileSystem::resolvePath(const String &decodedURI)
    {
        String path = decodedURI.substring(m_name.length());
        if (path.isEmpty()) path = "/";
        if (path != "/" && path.endsWith("/"))
            path = path.substring(0, path.length() - 1);
        return path;
    }

    bool WebDAVFileSystem::getQuota(QuotaSz &available, QuotaSz &used)
    {
        available = 0, used = 0;
        if (m_quotaCb)
        {
            m_quotaCb(m_fs, available, used);
            return true;
        }
        return false;
    }
}
