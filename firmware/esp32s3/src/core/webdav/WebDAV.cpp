#include "WebDAV.h"
#include "core/Strings.h"
#include <detail/mimetable.h>

#include <rom/miniz.h>
#define crc32(a, len) mz_crc32( 0xffffffff,(const unsigned char *)a, len)

namespace core
{
    void WebDAV::begin(WebServer &server)
    {
        const char* hdrs[] = { "Depth", "Destination", "Overwrite" };
        m_server = &server;
        m_server->addHandler(this);
        m_server->collectHeaders(hdrs, 3);
        m_mounted.clear();
    }

    void WebDAV::addFS(fs::FS& fs, const String& mountPoint, const String& alias, QuotaCb quotaCb)
    {
        m_mounted.push_back({ &fs, mountPoint, alias, quotaCb });
    }

    bool WebDAV::canHandle(HTTPMethod method, String uri)
    {
        switch(method)
        {
            case HTTP_OPTIONS:
            case HTTP_GET:
            case HTTP_PUT:
            case HTTP_DELETE:
            case HTTP_COPY:
            case HTTP_MKCOL:
            case HTTP_MOVE:
            case HTTP_PROPFIND:
                return true;
        }
        return false;
    }

    bool WebDAV::handle(WebServer& server, HTTPMethod method, String uri)
    {
        switch(method)
        {
            case HTTP_OPTIONS:  return handleOPTIONS();
            case HTTP_GET:      return handleGET();
            case HTTP_PUT:      return handlePUT();
            case HTTP_DELETE:   return handleDELETE();
            case HTTP_COPY:     return handleCOPY();
            case HTTP_MKCOL:    return handleMKCOL();
            case HTTP_MOVE:     return handleMOVE();
            case HTTP_PROPFIND: return handlePROPFIND();
        }
        return false;
    }

    bool WebDAV::handleOPTIONS()
    {
        log_i("handleOPTIONS");

        m_server->sendHeader("DAV", "1");
        m_server->sendHeader("Allow", "OPTIONS, GET, PROPFIND, PUT, DELETE, MKCOL, COPY, MOVE");
        return send200OK();
    }

    bool WebDAV::handleGET()
    {
        // check if request can be processed
        String uri = decodeURI(m_server->uri());
        FS* fs = getMountedFS(uri);
        if (!fs) return false;

        log_i("handleGET");
        m_server->send(501);
        return true;
    }

    bool WebDAV::handlePUT()    
    { 
        // check if request can be processed
        String uri = decodeURI(m_server->uri());
        FS* fs = getMountedFS(uri);
        if (!fs) return false;

        log_i("handlePUT"); 
        m_server->send(501);
        return true;
    }

    bool WebDAV::handleDELETE()
    { 
        // check if request can be processed
        String uri = decodeURI(m_server->uri());
        FS* fs = getMountedFS(uri);
        if (!fs) return false;

        log_i("handleDELETE");
        m_server->send(501);
        return true;
    }

    bool WebDAV::handleCOPY() 
    {
        // check if request can be processed
        String uri = decodeURI(m_server->uri());
        FS* fs = getMountedFS(uri);
        if (!fs) return false;

        log_i("handleCOPY");
        m_server->send(501);
        return true;
    }

    bool WebDAV::handleMKCOL()
    {   
        // check if request can be processed
        String uri = decodeURI(m_server->uri());
        FS* fs = getMountedFS(uri);
        if (!fs) return false;

        // check if resource is NOT available
        String path = getFilePath(fs->mountPoint, uri);
        Resource resource = getResource(fs, path);

        log_i("handleMKCOL");
        log_i("uri: %s", uri.c_str());
        log_i("filesystem: %s (%s)", fs->mountPoint.c_str(), fs->alias.c_str());
        log_i("path: %s", path.c_str());

        if (resource != Resource::None)
            return send405MethodNotAllowed();

        // check if parent is also a directory
        int parentIdx = path.lastIndexOf('/');
        if (parentIdx > 0)
        {
            String parentPath = path.substring(0, parentIdx);
            fs::File parentFile = fs->underlying->open(parentPath, "r");
            if (!parentFile.isDirectory())
                return send409Conflict();
        }

        // try to create directory
        if (!fs->underlying->mkdir(path))
            return send507InsufficientStorage();
        return send201Created();
    }

    bool WebDAV::handleMOVE()
    { 
        // check if request can be processed
        String uri = decodeURI(m_server->uri());
        FS* fs = getMountedFS(uri);
        if (!fs) return false;

        log_i("handleMOVE");
        m_server->send(501);
        return true;
    }

    bool WebDAV::handlePROPFIND()
    {
        std::vector<ResourceProps> resources;
        String uri = decodeURI(m_server->uri());
        if (uri == "/")
        {
            log_i("handlePROPFIND (mounts)");

            // collect the list of mounted file systems
            resources.emplace_back(uri, time(nullptr), "");
            if (getDepth() == Depth::Child)
            {
                for (auto& fs : m_mounted)
                {
                    if (fs::File childFile = fs.underlying->open("/"))
                    {
                        resources.emplace_back(
                            getFileURI(fs.mountPoint, childFile),
                            childFile.getLastWrite(),
                            fs.alias);
                        if (fs.quotaCb)
                        {
                            QuotaSz available = 0, used = 0;
                            fs.quotaCb(*fs.underlying, available, used);
                            resources.back().setQuota(available, used);
                        }
                        childFile.close();
                    }
                }
            }
        }
        else
        {
            // check if request can be processed
            FS* fs = getMountedFS(uri);
            if (!fs) return false;

            // check if resource available
            String path = getFilePath(fs->mountPoint, uri);
            Resource resource = getResource(fs, path);

            log_i("handlePROPFIND (resources)");
            log_i("uri: %s", uri.c_str());
            log_i("filesystem: %s (%s)", fs->mountPoint.c_str(), fs->alias.c_str());
            log_i("path: %s", path.c_str());

            if (resource == Resource::None) 
                return send404NotFound();

            // collect the list of resources
            fs::File baseFile = fs->underlying->open(path, "r");
            resources.emplace_back(
                getFileURI(fs->mountPoint, baseFile),
                baseFile.getLastWrite(),
                baseFile.size());

            if (resource == Resource::Dir && getDepth() == Depth::Child)
            {
                fs::File dir = fs->underlying->open(path);
                while (fs::File childFile = dir.openNextFile())
                {
                    resources.emplace_back(
                        getFileURI(fs->mountPoint, childFile),
                        childFile.getLastWrite(),
                        childFile.size());
                    childFile.close();
                }
                dir.close();
            }
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

    ////////////////////////////////////////////////////////////////////////////

    String WebDAV::decodeURI(const String& encoded)
    {
        return WebServer::urlDecode(encoded);
    }

    // TODO: check implementation!
    String WebDAV::encodeURI(const String& decoded)
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

    String WebDAV::getDateString(time_t date)
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

    String WebDAV::getContentType(const String& uri)
    {
        for (const auto& e : mime::mimeTable)
            if (uri.endsWith(e.endsWith)) return e.mimeType;
        return mime::mimeTable[mime::none].mimeType;
    }

    String WebDAV::getETag(const String &uri, time_t modified)
    {
        char buf[uri.length() + 32];
        sprintf(buf, "%s%lu", uri.c_str(), (unsigned long)modified);
        uint32_t crc = crc32(buf, strlen(buf));
        sprintf(buf, "\"%08x\"", crc);
        return buf;
    }

    ////////////////////////////////////////////////////////////////////////////

    String WebDAV::buildProp(const String &prop, const String &val)
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

    String WebDAV::buildOptProp(const String &prop, const String &val)
    {
        if (val.isEmpty()) return "";
        return buildProp(prop, val);
    }

    String WebDAV::getFileURI(const String& mountPoint, fs::File &file)
    {
        String path = file.path();
        if (!path.startsWith("/")) path = "/" + path;
        if (file.isDirectory() && !path.endsWith("/")) path += "/";
        return (mountPoint + path);
    }

    String WebDAV::getFilePath(const String& mountPoint, const String& uri)
    {
        String path = uri.substring(mountPoint.length());
        if (path.isEmpty()) path = "/";
        if (path != "/" && path.endsWith("/"))
            path = path.substring(0, path.length() - 1);
        return path;
    }

    ////////////////////////////////////////////////////////////////////////////

    WebDAV::FS* WebDAV::getMountedFS(const String &uri)
    {
        for (auto& fs : m_mounted)
            if (uri.startsWith(fs.mountPoint)) return &fs;
        return nullptr;
    }

    WebDAV::Resource WebDAV::getResource(FS* mounted, const String& path)
    {
        auto res = Resource::None;
        if (mounted)
        {
            fs::File file = mounted->underlying->open(path, "r");
            if (file)
            {
                res = file.isDirectory() ? Resource::Dir : Resource::File;
                file.close();
            }
        }
        return res;
    }

    WebDAV::Depth WebDAV::getDepth()
    {
        const auto depth = m_server->header("Depth");
        log_i("header depth: %s", depth.c_str());
        if (!depth.isEmpty())
        {
            if (depth == "1") return Depth::Child;
            if (depth == "infinity") return Depth::All;
        }
        return Depth::None;
    }

    bool WebDAV::send200OK()
        { m_server->send(200); return true; }
    bool WebDAV::send201Created()
        { m_server->send(201); return true; }
    bool WebDAV::send404NotFound() 
        { m_server->send(404); return true; }
    bool WebDAV::send405MethodNotAllowed() 
        { m_server->send(405); return true; }
    bool WebDAV::send409Conflict() 
        { m_server->send(409); return true; }
    bool WebDAV::send507InsufficientStorage() 
        { m_server->send(507); return true; }

    ///////////////////////////////////////////////////////

    WebDAV::ResourceProps::ResourceProps(const String &uri, time_t modified, const String &name)
        : m_href(encodeURI(uri))
        , m_lastModified(getDateString(modified))
        , m_displayName(name)
        , m_etag(getETag(uri, modified))
    {
        if (uri.endsWith("/"))
            m_resourceType = "<D:collection/>";
    }

    WebDAV::ResourceProps::ResourceProps(const String &uri, time_t modified, size_t size)
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

    void WebDAV::ResourceProps::setQuota(unsigned long available, unsigned long used)
    {
        m_availableBytes = String(available);
        m_usedBytes = String(used);
    }

    String WebDAV::ResourceProps::toString() const
    {
        return
        buildProp("response",
            buildProp("href", m_href) +
            buildProp("propstat",
                buildProp("prop",
                    buildProp   ("resourcetype", m_resourceType) +
                    buildOptProp("displayname", m_displayName) +
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
}
