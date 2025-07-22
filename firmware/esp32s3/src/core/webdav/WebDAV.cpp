#include "WebDAV.h"
#include "core/Strings.h"
#include <detail/mimetable.h>

#include <rom/miniz.h>
#define crc32(a, len) mz_crc32( 0xffffffff,(const unsigned char *)a, len)

namespace core
{
    void WebDAV::begin(WebServer& server, fs::FS& fs, const String& mountPoint)
    {
        m_start = time(nullptr);

        m_fs = &fs;
        m_ws = &server;
        m_mp = mountPoint;
        m_ws->addHandler(this);

        const char* hdrs[] = { "Depth", "Destination", "Overwrite" };
        m_ws->collectHeaders(hdrs, 3);
    }

    bool WebDAV::canHandle(HTTPMethod method, String uri)
    {
        log_i("canHandle:\nmethod: %d\nuri: %s", int(method), uri.c_str());
        if (uri.startsWith(m_mp))
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
        }
        return false;
    }

    bool WebDAV::handle(WebServer& server, HTTPMethod method, String uri)
    {
        log_i("handle:\nmethod: %d\nuri: %s", int(method), uri.c_str());

        m_ws->sendHeader("DAV", "1");
        switch(method)
        {
            case HTTP_OPTIONS:  handleOPTIONS();  break;
            case HTTP_GET:      handleGET();      break;
            case HTTP_PUT:      handlePUT();      break;
            case HTTP_DELETE:   handleDELETE();   break;
            case HTTP_COPY:     handleCOPY();     break;
            case HTTP_MKCOL:    handleMKCOL();    break;
            case HTTP_MOVE:     handleMOVE();     break;
            case HTTP_PROPFIND: handlePROPFIND(); break;
            default:
                m_ws->send(404);
                return false;
        }
        return true;
    }

    void WebDAV::handleOPTIONS()
    {
        log_i("handleOPTIONS");

        m_ws->sendHeader("Allow", "OPTIONS, GET, PROPFIND, PUT, DELETE, MKCOL, COPY, MOVE");
        m_ws->send(200);
    }

    void WebDAV::handleGET()
    {
        log_i("handleGET");
        m_ws->send(501);
    }

    void WebDAV::handlePUT()    
    { 
        log_i("handlePUT"); 
        m_ws->send(501);
    }

    void WebDAV::handleDELETE()
    { 
        log_i("handleDELETE");
        m_ws->send(501); 
    }

    void WebDAV::handleCOPY() 
    {
        log_i("handleCOPY");
        m_ws->send(501);
    }

    void WebDAV::handleMKCOL()
    {   
        log_i("handleMKCOL");
        m_ws->send(501); 
    }

    void WebDAV::handleMOVE()
    { 
        log_i("handleMOVE");
        m_ws->send(501);
    }

    void WebDAV::handlePROPFIND()
    {
        log_i("handlePROPFIND");

        // check if resource available
        auto path = getFilePath(m_mp, decodeURI(m_ws->uri()));
        auto resource = getResource(path);
        if (resource == Resource::None) return m_ws->send(404);

        // collect the list of resources
        std::vector<Props> resources;
        fs::File baseFile = m_fs->open(path, "r");
        resources.emplace_back(
            getFileURI(m_mp, baseFile),
            baseFile.getLastWrite(),
            baseFile.size());

        auto depth = getDepth();
        if (resource == Resource::Dir && depth == Depth::Child)
        {
            fs::File dir = m_fs->open(path);
            while (fs::File childFile = dir.openNextFile())
            {
                resources.emplace_back(
                    getFileURI(m_mp, childFile),
                    childFile.getLastWrite(),
                    childFile.size());
                childFile.close();
            }
            dir.close();
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
        m_ws->setContentLength(contentLength);
        m_ws->send(207, "text/xml; charset=\"utf-8\"", "");
        m_ws->sendContent(xmlPreamble);
        for (const auto& resource : resources)
        {
            m_ws->sendContent(resource.toString());
        }
        m_ws->sendContent(xmlEpilogue);
    }


    // void WebDAV::handlePROPFIND()
    // {
    //     log_i("handlePROPFIND");

    //     auto path = getFSPath();
    //     auto resource = getResource(path);
    //     if (resource == Resource::None) return m_ws->send(404);
 
    //     m_ws->setContentLength(CONTENT_LENGTH_UNKNOWN);
    //     m_ws->send(207, "application/xml;charset=utf-8", "");
    //     m_ws->sendContent("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
    //     m_ws->sendContent("<D:multistatus xmlns:D=\"DAV:\">");

    //     auto depth = getDepth();
    //     fs::File baseFile = m_fs->open(path, "r");
    //     sendPropResponse(baseFile);

    //     log_i("path: %s\nresource: %d\ndepth: %d", path.c_str(), int(resource), int(depth));
    //     if (resource == Resource::Dir && depth == Depth::Child)
    //     {
    //         fs::File dir = m_fs->open(path);
    //         log_i("read children for: %s", dir.path());
    //         while (fs::File childFile = dir.openNextFile())
    //         {
    //             log_i("child: %s", childFile.path());
    //             sendPropResponse(childFile);
    //             childFile.close();
    //         }
    //         dir.close();
    //     }

    //     baseFile.close();
    //     m_ws->sendContent("</D:multistatus>");

    //     // m_ws->sendHeader("Content-Type", "application/xml; charset=\"utf-8\"");
    //     // m_ws->sendHeader("Content-Length", String(body.length()));
    //     // m_ws->send(207, "application/xml", body);
    // }

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

    String WebDAV::getFSPath()
    {
        String uri = decodeURI(m_ws->uri());
        String path = uri.substring(m_mp.length());
        if (path.isEmpty()) path = "/";
        if (path != "/" && path.endsWith("/"))
            path = path.substring(0, path.length() - 1);
        return path;
    }

    WebDAV::Resource WebDAV::getResource(const String& fsPath)
    {
        auto res = Resource::None;
        fs::File file = m_fs->open(fsPath, "r");
        if (file)
        {
            res = file.isDirectory() ? Resource::Dir : Resource::File;
            file.close();
        }
        return res;
    }

    WebDAV::Depth WebDAV::getDepth()
    {
        const auto depth = m_ws->header("Depth");
        log_i("header depth: %s", depth.c_str());
        if (!depth.isEmpty())
        {
            if (depth == "1") return Depth::Child;
            if (depth == "infinity") return Depth::All;
        }
        return Depth::None;
    }

    // void WebDAV::sendPropResponse(fs::File& file)
    // {
    //     String uri = file.path();
    //     if (!uri.startsWith("/")) uri = "/" + uri;
    //     if (file.isDirectory() && !uri.endsWith("/")) uri += "/";
    //     uri = m_mp + uri;
    //     log_i("uri: %s", uri.c_str());
    //     m_ws->sendContent(
    //         "<D:response>");
    //     sendPropContent(
    //         "href", core::c2enc(uri));
    //     m_ws->sendContent(
    //         "<D:propstat>"
    //         "<D:prop>");
    //     time_t lastWrite = file.getLastWrite();
    //     sendPropContent("getlastmodified", toString(lastWrite));
    //     if (file.isDirectory())
    //         sendPropContent("resourcetype", "<D:collection/>");
    //     else
    //     {
    //         sendPropContent("resourcetype", "");
    //         sendPropContent("getcontentlength", String(file.size()));
    //         sendPropContent("getcontenttype", getContentType(uri));
    //         sendPropContent("getetag", getETag(uri, lastWrite));
    //     }
    //     m_ws->sendContent(
    //         "</D:prop>"
    //         "<D:status>HTTP/1.1 200 OK</D:status>"
    //         "</D:propstat>"
    //         "</D:response>");
    // }

    // void WebDAV::sendPropContent(const String &prop, const String &content)
    // {
    //     if (content.isEmpty())
    //         m_ws->sendContent("<D:" + prop + "/>");
    //     else
    //     {
    //         char buf[2 * prop.length() + content.length() + 16];
    //         snprintf(buf, sizeof(buf), "<D:%s>%s</D:%s>",
    //             prop.c_str(), content.c_str(), prop.c_str());
    //         m_ws->sendContent(buf, strlen(buf));
    //     }
    // }

    ///////////////////////////////////////////////////////

    // <D:response>
    //   <D:href>/sdcard/folder1/</D:href>
    //   <D:propstat>
    //     <D:prop>
    //       <D:resourcetype><D:collection/></D:resourcetype>
    //       <D:displayname>folder1</D:displayname>
    //       <D:getlastmodified>Mon, 21 Jul 2025 15:00:00 GMT</D:getlastmodified>
    //     </D:prop>
    //     <D:status>HTTP/1.1 200 OK</D:status>
    //   </D:propstat>
    // </D:response>

    // <D:response>
    //   <D:href>/sdcard/file1.txt</D:href>
    //   <D:propstat>
    //     <D:prop>
    //       <D:resourcetype/>
    //       <D:getcontentlength>1234</D:getcontentlength>
    //       <D:getcontenttype>text/plain; charset=UTF-8</D:getcontenttype>
    //       <D:getlastmodified>Mon, 21 Jul 2025 15:00:00 GMT</D:getlastmodified>
    //     </D:prop>
    //     <D:status>HTTP/1.1 200 OK</D:status>
    //   </D:propstat>
    // </D:response>

    WebDAV::Props::Props(const String &uri, time_t modified, const String &name)
        : m_href(encodeURI(uri))
        , m_lastModified(getDateString(modified))
        , m_displayName(name)
        , m_etag(getETag(uri, modified))
    {
        if (uri.endsWith("/"))
            m_resourceType = "<D:collection/>";
    }

    WebDAV::Props::Props(const String &uri, time_t modified, size_t size)
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

    String WebDAV::Props::toString() const
    {
        return
        buildProp("response",
            buildProp("href", m_href) +
            buildProp("propstat",
                buildProp("prop",
                    buildProp("resourcetype", m_resourceType) +
                    buildOptProp("displayname", m_displayName) +
                    buildOptProp("getcontentlength", m_contentLength) +
                    buildOptProp("getcontenttype", m_contentType) +
                    buildOptProp("getetag", m_etag) +
                    buildProp("getlastmodified", m_lastModified)
                ) +
                buildProp("status", "HTTP/1.1 200 OK")
            )
        );
    }
}
