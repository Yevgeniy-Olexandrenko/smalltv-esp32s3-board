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

    String Server::getHttpDateTime(time_t timestamp) const
    {
        char buf[40];
        struct tm* tmstruct = gmtime(&timestamp);
        strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tmstruct);
        return String(buf);
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

    bool FileSystem::deleteRecursive(const String &path)
    {
        if (fs::File entry = m_fs.open(path))
        {
            if (!entry.isDirectory())
            {
                entry.close();
                return m_fs.remove(path);
            }
            while (fs::File child = entry.openNextFile())
            {
                deleteRecursive(path + "/" + child.name());
                child = entry.openNextFile();
            }
            entry.close();
            return m_fs.rmdir(path);
        }
        return false;
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
        , m_lastModified(handler.m_server.getHttpDateTime(modified))
    {
        if (uri.endsWith("/"))
            m_resourceType = "<D:collection/>";
    }

    Handler::Resource::Resource(Handler &handler, const String &uri, time_t modified, size_t size)
        : m_href(handler.m_server.encodeURI(uri))
        , m_lastModified(handler.m_server.getHttpDateTime(modified))
        , m_etag(handler.getETag(size, modified))
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
                FileSystem* fs = getMountedFS(decodedURI);
                return (fs != nullptr);
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

        FileSystem* fs = getMountedFS(decodedURI);
        if (fs)
        {
            String path = fs->resolvePath(decodedURI);
            String destination = server.header("Destination");
            switch(method)
            {
                case HTTP_MKCOL:  handleMKCOL  (*fs, path); break;
                case HTTP_DELETE: handleDELETE (*fs, path); break;
                case HTTP_GET:    handleGET    (*fs, path); break;
                case HTTP_PUT:    handlePUT    (*fs, path); break;
                case HTTP_COPY:   handleCOPY   (*fs, path, destination); break;
                case HTTP_MOVE:   handleMOVE   (*fs, path, destination); break;
            }
            return true;
        }        
        return false;
    }

    FileSystem* Handler::getMountedFS(const String &uri)
    {
        for (auto& fs : m_mountedFS)
            if (uri.startsWith(fs.getName())) return &fs;
        return nullptr;
    }

    String Handler::getETag(size_t size, time_t modified) const
    {
        return ('"' + String(size) + '-' + String(modified) + '"');
    }

    // done
    void Handler::handleOPTIONS()
    {
        log_i("OPTIONS");
        m_server.sendHeader("DAV", "1");
        m_server.sendHeader("Allow", "OPTIONS, GET, PROPFIND, PUT, DELETE, MKCOL, COPY, MOVE");
        m_server.sendCode(200, "OK");
    }

    // done
    bool Handler::handlePROPFIND(const String& decodedURI)
    {
        log_i("PROPFIND: %s", decodedURI.c_str());

        bool depthChild = (m_server.getHeader("Depth") == "1");
        std::vector<Resource> resources;
        if (decodedURI == "/")
        {
            // collect the list of mounted file systems
            resources.emplace_back(*this, decodedURI, time(nullptr));
            if (depthChild)
            {
                for (auto& fs : m_mountedFS)
                {
                    if (fs::File childFile = fs->open("/"))
                    {
                        resources.emplace_back(
                            *this, fs.resolveURI(childFile), childFile.getLastWrite());
                        childFile.close();

                        FileSystem::QuotaSz available, used;
                        if (fs.getQuota(available, used))
                            resources.back().setQuota(available, used);
                    }
                }
            }
        }
        else
        {
            // check if request can be processed
            FileSystem* fs = getMountedFS(decodedURI);
            if (!fs) return false;

            // check if resource available
            String path = fs->resolvePath(decodedURI);
            if (!(*fs)->exists(path))
            {
                m_server.sendCode(404, "Not found");
                return true;
            }

            // collect the list of resources
            fs::File baseFile = (*fs)->open(path);
            resources.emplace_back(
                *this, fs->resolveURI(baseFile), baseFile.getLastWrite(), baseFile.size());
            if (baseFile.isDirectory() && depthChild)
            {
                while (fs::File childFile = baseFile.openNextFile())
                {
                    resources.emplace_back(
                        *this, fs->resolveURI(childFile), childFile.getLastWrite(), childFile.size());
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
    void Handler::handleMKCOL(FileSystem& fs, const String& path)
    {   
        log_i("MKCOL: %s : %s", fs.getName().c_str(), path.c_str());

        // check that the request should not have a body
        if (m_server.getContentLength() > 0)
            return m_server.sendCode(415, "MKCOL with body not supported");

        // check if resource is NOT available
        if (fs->exists(path)) 
            return m_server.sendCode(405, "File or directory already exists");

        // check if parent directory exists
        int slashIdx = path.lastIndexOf('/');
        String parent = (slashIdx > 0) ? path.substring(0, slashIdx) : "/";
        if (!fs->exists(parent)) 
            return m_server.sendCode(409, "Parent directory does not exist");

        // trying to create the directory
        if (!fs->mkdir(path)) 
            return m_server.sendCode(500, "Failed to create directory");
        m_server.sendCode(201, "Directory created");
    }

    void Handler::handleDELETE(FileSystem& fs, const String& path)
    { 
        log_i("DELETE: %s : %s", fs.getName().c_str(), path.c_str());
        
        // do not remove the root
        if (path == "/")
            return m_server.sendCode(403, "Forbidden: can't delete root");

        // check if an object exists
        if (!fs->exists(path))
            return m_server.sendCode(404, "Not found");

        // determine whether it is a file or a directory
        File file = fs->open(path);
        if (!file)
            m_server.sendCode(500, "Cannot open file/dir");
        bool isDir = file.isDirectory();
        file.close();

        // trying to delete and check the result
        if (!(isDir ? fs.deleteRecursive(path) : fs->remove(path)))
            return m_server.sendCode(500, "Delete failed");
        return m_server.sendCode(204, "Deleted");
    }

    void Handler::handleGET(FileSystem& fs, const String& path)
    {
        log_i("GET: %s : %s", fs.getName().c_str(), path.c_str());
        m_server.sendCode(501, "Not implemented");
    }

    void Handler::handlePUT(FileSystem& fs, const String& path)    
    { 
        log_i("PUT: %s : %s", fs.getName().c_str(), path.c_str()); 
        m_server.sendCode(501, "Not implemented");
    }

    void Handler::handleCOPY(FileSystem& fs, const String& path, const String& dest) 
    {
        log_i("COPY: %s : %s -> %s", fs.getName().c_str(), path.c_str(), dest.c_str());
        m_server.sendCode(501, "Not implemented");
    }

    void Handler::handleMOVE(FileSystem& fs, const String& path, const String& dest)
    { 
        log_i("MOVE: %s : %s -> %s", fs.getName().c_str(), path.c_str(), dest.c_str());
        m_server.sendCode(501, "Not implemented");
    }   
}
