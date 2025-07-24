#include <rom/miniz.h>
#include <detail/mimetable.h>
#include "WebDAV.h"

#define crc32(a, len) mz_crc32(0xffffffff, (const unsigned char *)a, len)

namespace WebDAV
{
    void Server::setHandler(Handler &handler)
    {
        static const char* hdrs[] = 
        {
            "Depth", "Destination", "Overwrite", "If-None-Match", "If-Modified-Since", "Range"
        };

        RequestHandler& requestHandler = static_cast<RequestHandler&>(handler);
        m_server->collectHeaders(hdrs, sizeof(hdrs) / sizeof(char*));
        m_server->addHandler(&requestHandler); 
    }

    String Server::decodeURI(const String &encodedURI) const
    {
        return m_server->urlDecode(encodedURI);
    }

    String Server::encodeURI(const String &decodedURI) const
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

    String Server::getContentType(const String& uri) const
    {
        for (const auto& e : mime::mimeTable)
            if (uri.endsWith(e.endsWith)) return e.mimeType;
        return mime::mimeTable[mime::none].mimeType;
    }

    ////////////////////////////////////////////////////////////////////////////

    String FileSystem::resolveURI(fs::File &file)
    {
        String path = file.path();
        if (!path.startsWith("/")) path = "/" + path;
        if (file.isDirectory() && !path.endsWith("/")) path += "/";
        return (m_name + path);
    }

    String FileSystem::resolvePath(const String &decodedURI)
    {
        String path = decodedURI.substring(m_name.length());
        if (path.isEmpty()) path = "/";
        if (path != "/" && path.endsWith("/"))
            path = path.substring(0, path.length() - 1);
        return path;
    }

    String FileSystem::convertTimestamp(time_t timestamp)
    {
        static const char *months[] = 
        {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

        static const char *wdays[] = 
        {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
        };

        // get & convert time to required format
        // Tue, 13 Oct 2015 17:07:35 GMT
        tm* gTm = gmtime(&timestamp);
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

    String FileSystem::generateETag(const String &uri, time_t modified, size_t size)
    {
        char buf[uri.length() + 32];
        snprintf(buf, sizeof(buf), "%s%ld%u",
            uri.c_str(), 
            static_cast<long>(modified),
            static_cast<unsigned int>(size));
        uint32_t crc = crc32(buf, strlen(buf));
        snprintf(buf, sizeof(buf), "\"%08x\"", crc);
        return buf;
    }

    bool FileSystem::getQuota(QuotaSz &available, QuotaSz &used)
    {
        available = 0, used = 0;
        if (m_quotaCb)
        {
            m_quotaCb(m_fs, available, used);
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////

    Handler::Resource::Resource(Handler &handler, const String &uri, time_t modified)
        : m_href(handler.m_server.encodeURI(uri))
        , m_lastModified(FileSystem::convertTimestamp(modified))
        , m_etag(FileSystem::generateETag(uri, modified, 0))
    {
        if (uri.endsWith("/"))
            m_resourceType = "<D:collection/>";
    }

    Handler::Resource::Resource(Handler &handler, const String &uri, time_t modified, size_t size)
        : m_href(handler.m_server.encodeURI(uri))
        , m_lastModified(FileSystem::convertTimestamp(modified))
        , m_etag(FileSystem::generateETag(uri, modified, size))
    {
        if (uri.endsWith("/"))
            m_resourceType = "<D:collection/>";
        else
        {
            m_contentLength = String(size);
            m_contentType = handler.m_server.getContentType(uri);
        }
    }

    void Handler::Resource::setQuota(FileSystem::QuotaSz available, FileSystem::QuotaSz used)
    {
        m_availableBytes = String(available);
        m_usedBytes = String(used);
    }

    String Handler::Resource::toString() const
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

    String Handler::Resource::buildProp(const String &prop, const String &val) const
    {
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

    String Handler::Resource::buildOptProp(const String &prop, const String &val) const
    {
        if (val.isEmpty()) return "";
        return buildProp(prop, val);
    }

    ////////////////////////////////////////////////////////////////////////////

    void Handler::begin(const Server& server)
    {
        m_server = server;
        m_server.setHandler(*this);
        m_mountedFS.clear();
    }

    void Handler::addFS(fs::FS& fs, const String& mountName, FileSystem::QuotaCb quotaCb)
    {
        if (!mountName.isEmpty())
        {
            if (mountName.startsWith("/"))
                m_mountedFS.emplace_back(fs, mountName, quotaCb);
            else
                m_mountedFS.emplace_back(fs, "/" + mountName, quotaCb);
        }
    }

    bool Handler::canHandle(HTTPMethod method, String uri)
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
                String decodedURI = m_server.decodeURI(uri);
                FileSystem* wdfs = getMountedFS(decodedURI);
                return (wdfs != nullptr);
        }
        return false;
    }

    bool Handler::handle(WebServer& server, HTTPMethod method, String uri)
    {
        if (method == HTTP_OPTIONS)
        {
            handleOPTIONS();
            return true;
        }
        
        String decodedURI = m_server.decodeURI(uri);
        if (method == HTTP_PROPFIND)
        {
            return handlePROPFIND(decodedURI);
        }

        FileSystem* wdfs = getMountedFS(decodedURI);
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

    FileSystem* Handler::getMountedFS(const String &uri)
    {
        for (auto& wdfs : m_mountedFS)
            if (uri.startsWith(wdfs.getName())) return &wdfs;
        return nullptr;
    }

    // done
    void Handler::handleOPTIONS()
    {
        log_i("handleOPTIONS");
        m_server.sendHeader("DAV", "1");
        m_server.sendHeader("Allow", "OPTIONS, GET, PROPFIND, PUT, DELETE, MKCOL, COPY, MOVE");
        m_server.sendCode(200, "OK");
    }

    // done
    bool Handler::handlePROPFIND(const String& decodedURI)
    {
        bool depthChild = (m_server.getHeader("Depth") == "1");
        std::vector<Resource> resources;
        if (decodedURI == "/")
        {
            log_i("handlePROPFIND (root)");

            // collect the list of mounted file systems
            resources.emplace_back(*this, decodedURI, time(nullptr));
            if (depthChild)
            {
                for (auto& wdfs : m_mountedFS)
                {
                    if (fs::File childFile = wdfs->open("/"))
                    {
                        resources.emplace_back(
                            *this, wdfs.resolveURI(childFile), childFile.getLastWrite());
                        childFile.close();

                        FileSystem::QuotaSz available, used;
                        if (wdfs.getQuota(available, used))
                            resources.back().setQuota(available, used);
                    }
                }
            }
        }
        else
        {
            // check if request can be processed
            FileSystem* wdfs = getMountedFS(decodedURI);
            if (!wdfs) return false;

            log_i("handlePROPFIND (resources)");

            // check if resource available
            String path = wdfs->resolvePath(decodedURI);
            if (!(*wdfs)->exists(path))
            {
                m_server.sendCode(404, "Not found");
                return true;
            }

            // collect the list of resources
            fs::File baseFile = (*wdfs)->open(path);
            resources.emplace_back(
                *this, wdfs->resolveURI(baseFile), baseFile.getLastWrite(), baseFile.size());
            if (baseFile.isDirectory() && depthChild)
            {
                while (fs::File childFile = baseFile.openNextFile())
                {
                    resources.emplace_back(
                        *this, wdfs->resolveURI(childFile), childFile.getLastWrite(), childFile.size());
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
        m_server.setContentLength(contentLength);
        m_server.sendCode(207, "text/xml; charset=\"utf-8\"", "");
        m_server.sendContent(xmlPreamble);
        for (const auto& resource : resources)
            m_server.sendContent(resource.toString());
        m_server.sendContent(xmlEpilogue);
        return true;
    }

    // done
    void Handler::handleMKCOL(FileSystem& wdfs, const String& path)
    {   
        log_i("handleMKCOL");

        // check that the request should not have a body
        if (m_server.getContentLength() > 0)
            return m_server.sendCode(415, "MKCOL with body not supported");

        // check if resource is NOT available
        if (wdfs->exists(path)) 
            return m_server.sendCode(405, "File or directory already exists");

        // check if parent directory exists
        int slashIdx = path.lastIndexOf('/');
        String parent = (slashIdx > 0) ? path.substring(0, slashIdx) : "/";
        if (!wdfs->exists(parent)) 
            return m_server.sendCode(409, "Parent directory does not exist");

        // trying to create the directory
        if (!wdfs->mkdir(path)) 
            return m_server.sendCode(500, "Failed to create directory");
        m_server.sendCode(201, "Directory created");
    }

    void Handler::handleDELETE(FileSystem& wdfs, const String& path)
    { 
        log_i("handleDELETE");
        m_server.sendCode(501, "");
    }

    void Handler::handleGET(FileSystem& wdfs, const String& path)
    {
        log_i("handleGET");
        m_server.sendCode(501, "");
    }

    void Handler::handlePUT(FileSystem& wdfs, const String& path)    
    { 
        log_i("handlePUT"); 
        m_server.sendCode(501, "");
    }

    void Handler::handleCOPY(FileSystem& wdfs, const String& path, const String& dest) 
    {
        log_i("handleCOPY");
        m_server.sendCode(501, "");
    }

    void Handler::handleMOVE(FileSystem& wdfs, const String& path, const String& dest)
    { 
        log_i("handleMOVE");
        m_server.sendCode(501, "");
    }   
}
