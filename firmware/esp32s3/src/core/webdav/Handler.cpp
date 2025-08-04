#include <detail/mimetable.h>
#include "Handler.h"

namespace WebDAV
{
    void Server::setHandler(Handler &handler)
    {
        static const char* hdrs[] = 
        {
            "Content-Length", "Transfer-Encoding", "Expect"
            "Depth", "Destination", "Overwrite", 
            "If-None-Match", "If-Modified-Since"
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

    Handler::Resource::Resource(Handler &handler, const String &uri, time_t modified)
        : m_href(handler.m_server.encodeURI(uri))
        , m_lastModified(handler.m_server.getHttpDateTime(modified))
    {
        if (uri.endsWith("/"))
            m_resourceType = "<D:collection/>";
    }

    Handler::Resource::Resource(Handler &handler, const String &uri, time_t modified, FileSystem::QuotaSz size)
        : Resource(handler, uri, modified)
    {
        if (!uri.endsWith("/"))
        {
            m_contentLength = String(size);
            m_contentType = handler.m_server.getContentType(uri);
        }
        m_etag = handler.getETag(size, modified);
    }

    void Handler::Resource::setQuota(FileSystem::QuotaSz available, FileSystem::QuotaSz used)
    {
        m_availableBytes = String(available);
        m_usedBytes = String(used);
    }

    String Handler::Resource::toString() const
    {
        return
        reqProp("response",
            reqProp("href", m_href) +
            reqProp("propstat",
                reqProp("prop",
                    reqProp("resourcetype", m_resourceType) +
                    optProp("getcontentlength", m_contentLength) +
                    optProp("getcontenttype", m_contentType) +
                    optProp("getetag", m_etag) +
                    reqProp("getlastmodified", m_lastModified) +
                    optProp("quota-available-bytes", m_availableBytes) +
                    optProp("quota-used-bytes", m_usedBytes)
                ) +
                reqProp("status", "HTTP/1.1 200 OK")
            )
        );
    }

    String Handler::Resource::reqProp(const String &prop, const String &val) const
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

    String Handler::Resource::optProp(const String &prop, const String &val) const
    {
        if (val.isEmpty()) return "";
        return reqProp(prop, val);
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
            case HTTP_HEAD:
            case HTTP_GET:
            case HTTP_PUT:
            case HTTP_MOVE:
            case HTTP_COPY:
                String decodedURI = m_server.decodeURI(uri);
                FileSystem* fs = resolveFS(decodedURI);
                return (fs != nullptr);
        }
        return false;
    }

    bool Handler::canRaw(String uri)
    {
        log_i("canRaw: %d : %s", bool(m_uploadFile), uri.c_str());
        //return bool(m_uploadFile);
        return true;
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

        if (FileSystem* sfs = resolveFS(decodedURI))
        {
            String spath = sfs->resolvePath(decodedURI);
            switch(method)
            {
                case HTTP_MKCOL:
                    handleMKCOL(*sfs, spath);
                    return true;

                case HTTP_DELETE:
                    handleDELETE(*sfs, spath); 
                    return true;

                case HTTP_HEAD:
                case HTTP_GET:
                    handleGET_HEAD(*sfs, spath, method == HTTP_HEAD); 
                    return true;

                case HTTP_PUT:
                    handlePUT(*sfs, spath); 
                    return true;
            }

            // handle destination
            String destinationHeader = m_server.getHeader("Destination");
            if (!destinationHeader.isEmpty())
            {
                decodedURI = m_server.decodeURI(destinationHeader.substring(destinationHeader.indexOf('/', 8)));
                if (FileSystem* dfs = resolveFS(decodedURI))
                {
                    String dpath = dfs->resolvePath(decodedURI);

                    // handle overwrite
                    bool overwrite = true;
                    String overwriteHeader = m_server.getHeader("Overwrite");
                    if (!overwriteHeader.isEmpty()) 
                        overwrite = (overwriteHeader[0] == 'T' || overwriteHeader[0] == 't');

                    // handle operation
                    if (method == HTTP_MOVE)
                        handleMOVE(*sfs, spath, *dfs, dpath, overwrite);
                    else if (method == HTTP_COPY)
                        handleCOPY(*sfs, spath, *dfs, dpath, overwrite);
                    return true;
                }
            }
        }
        return false;
    }

    void Handler::raw(WebServer &server, String requestUri, HTTPRaw &raw)
    {
        log_i("raw: %d", raw.status);
        log_i("content-length: %d", server.clientContentLength());

        switch (raw.status)
        {
            case RAW_START:
                {
                log_i("RAW_START");

                log_i("=== Incoming HTTP Request ===");
                log_i("Method: %d : %s", server.method(), server.uri().c_str());
                for (int i = 0; i < server.headers(); ++i) 
                {
                    log_i("%s: %s", server.headerName(i).c_str(), server.header(i).c_str());
                }
                log_i("=============================");

                auto length   = m_server.getHeader("Content-Length").toInt();
                bool chunked  = m_server.getHeader("Transfer-Encoding").equalsIgnoreCase("chunked");

                log_i("length: %d", length);
                log_i("chunked: %d", chunked);

                m_server.sendCode(100, "");
                }
                break;

            case RAW_WRITE:
                log_i("RAW_WRITE");
                break;

            case RAW_END:
                log_i("RAW_END");
                break;

            case RAW_ABORTED:
                log_i("RAW_ABORTED");
                break;
        }
    }

    FileSystem* Handler::resolveFS(const String &uri)
    {
        for (auto& fs : m_mountedFS)
            if (uri.startsWith(fs.getName())) return &fs;
        return nullptr;
    }

    String Handler::getETag(FileSystem::QuotaSz size, time_t modified) const
    {
        return ('"' + String(size, 16) + '-' + String(modified, 16) + '"');
    }

    void Handler::handleOPTIONS()
    {
        log_i("OPTIONS");
        m_server.sendHeader("DAV", "1");
        m_server.sendHeader("Allow", "OPTIONS, PROPFIND, MKCOL, DELETE, HEAD, GET, PUT, MOVE, COPY");
        m_server.sendCode(200, "OK");
    }

    bool Handler::handlePROPFIND(const String& decodedURI)
    {
        bool depthChild = (m_server.getHeader("Depth") == "1");
        log_i("PROPFIND: depth = %d : %s", int(depthChild), decodedURI.c_str());

        // collect the list of resources
        std::vector<Resource> resources;
        if (decodedURI == "/")
        {
            // collect the list of mounted file systems
            time_t fakeModifyTime = time(nullptr);
            if (depthChild)
            {
                for (auto& fs : m_mountedFS)
                {
                    if (fs::File childFile = fs->open("/", FILE_READ))
                    {
                        FileSystem::QuotaSz free, used;
                        if (fs.getQuota(free, used))
                        {
                            resources.emplace_back(
                                *this, fs.resolveURI(childFile), fakeModifyTime, used);
                            resources.back().setQuota(free, used);
                        }
                        else
                        {
                            resources.emplace_back(
                                *this, fs.resolveURI(childFile), fakeModifyTime);
                        }
                    }
                }
            }
            resources.emplace_back(
                *this, decodedURI, fakeModifyTime);
        }
        else
        {
            // check if request can be processed
            FileSystem* fs = resolveFS(decodedURI);
            if (!fs) return false;

            // check if resource available
            String path = fs->resolvePath(decodedURI);
            if (!(*fs)->exists(path))
            {
                m_server.sendCode(404, "Not found");
                return true;
            }

            // collect the list of resources
            fs::File baseFile = (*fs)->open(path, FILE_READ);
            time_t modifyTime = baseFile.getLastWrite();
            size_t childCount = 0;
            if (baseFile.isDirectory() && depthChild)
            {
                while (fs::File childFile = baseFile.openNextFile())
                {
                    time_t childModifyTime = childFile.getLastWrite();
                    if (childModifyTime > modifyTime) 
                        modifyTime = childModifyTime;

                    resources.emplace_back(
                        *this, fs->resolveURI(childFile), childModifyTime, childFile.size());
                    childCount++;
                }
            }
            resources.emplace_back(
                *this, fs->resolveURI(baseFile), modifyTime, childCount);
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

    void Handler::handleMKCOL(FileSystem& fs, const String& path)
    {   
        log_i("MKCOL: %s : %s", fs.getName().c_str(), path.c_str());

        // check that the request should not have a body
        if (m_server.getContentLength() > 0)
            return m_server.sendCode(415, "MKCOL with body not supported");

        // check if resource is NOT available
        if (fs->exists(path)) 
            return m_server.sendCode(409, "File or directory already exists");

        // check if parent directory exists
        if (!handleParentExists(fs, path)) return;

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

        // trying to delete and check the result
        if (!FileSystem::removeFileDir(fs, path))
            return m_server.sendCode(500, "Delete failed");
        return m_server.sendCode(204, "Deleted");
    }

    void Handler::handleGET_HEAD(FileSystem& fs, const String& path, bool isHEAD)
    {
        if (isHEAD)
            log_i("HEAD: %s : %s", fs.getName().c_str(), path.c_str());
        else
            log_i("GET: %s : %s", fs.getName().c_str(), path.c_str());

        // check if an object exists
        if (!fs->exists(path))
            return m_server.sendCode(404, "Not found");

        // check if object is not a directory
        File file = fs->open(path, FILE_READ);
        if (!file || file.isDirectory()) 
            return m_server.sendCode(403, "Forbidden (is directory)");

        // collecting basic information about a file for caching
        String etag = getETag(file.size(), file.getLastWrite());
        String lastmod = m_server.getHttpDateTime(file.getLastWrite());
        String contentType = m_server.getContentType(path);

        // handling caching and checking file for modification
        if (m_server.getHeader("If-None-Match") == etag ||
            m_server.getHeader("If-Modified-Since") == lastmod)
        {
            m_server.sendHeader("ETag", etag);
            m_server.sendHeader("Last-Modified", lastmod);
            return m_server.sendCode(304, "");
        }

        // setting cache parameters and sending the file
        m_server.sendHeader("ETag", etag);
        m_server.sendHeader("Last-Modified", lastmod);
        m_server.sendHeader("Cache-Control", "private, max-age=0, must-revalidate");
        if (isHEAD)
        {
            m_server.setContentLength(file.size());
            m_server.sendCode(200, contentType, "");
        }
        else
            m_server.sendFile(file, contentType);
    }

    void Handler::handlePUT(FileSystem& fs, const String& path)    
    { 
        log_i("PUT: %s : %s", fs.getName().c_str(), path.c_str()); 
        
        // check that the distination is not a directory
        if (path.endsWith("/"))
            return m_server.sendCode(403, "Cannot PUT to directory");

        // ensure parent directory exists
        if (!handleParentExists(fs, path)) return;

        // try to open destination file
        // bool exists = fs->exists(path);
        // File file = fs->open(path, FILE_WRITE);
        // if (!file)
        //     return m_server.sendCode(500, "Failed to open file");
        
        // start pending upload logic
        // int contentLen = m_server.getWebServer().clientContentLength();
        // WiFiClient client = m_server.getWebServer().client();
        // log_i("content-length: %d", contentLen);

        // const size_t BUF_SIZE = 4096;
        // uint8_t* buf = (uint8_t*)malloc(BUF_SIZE);
        // if (!buf) {
        //     file.close();
        //     m_server.sendCode(500, "Memory allocation failed");
        //     return;
        // }

        // log_i("start copying");
        // int remaining = contentLen;
        // bool ok = true;
        // while (remaining > 0 && client.connected()) {
        //     size_t toRead = remaining < BUF_SIZE ? remaining : BUF_SIZE;
        //     int n = client.readBytes(buf, toRead);
        //     log_i("read: %d : %d", toRead, n);
        //     if (n <= 0) { ok = false; break; }
        //     int m = file.write(buf, n);
        //     log_i("write: %d : %d", n, m);
        //     if (m != n) { ok = false; break; }
        //     remaining -= n;
        // }

        // free(buf);
        // file.close();

        // log_i("result: %d", (ok && remaining == 0));
        // if (ok && remaining == 0)
        //     m_server.sendCode(fs->exists(path) ? 204 : 201, "File uploaded");
        // else
        //     m_server.sendCode(500, "Upload failed");

        auto length   = m_server.getHeader("Content-Length").toInt();
        bool chunked  = m_server.getHeader("Transfer-Encoding").equalsIgnoreCase("chunked");

        log_i("length: %d", length);
        log_i("chunked: %d", chunked);

        if (!length || chunked)
            m_server.sendCode(411, "");
    }

    void Handler::handleMOVE(FileSystem &sfs, const String &spath, FileSystem &dfs, const String &dpath, bool overwrite)
    {
        log_i("MOVE: %s : %s -> %s : %s", 
            sfs.getName().c_str(), spath.c_str(), 
            dfs.getName().c_str(), dpath.c_str());

        // check source and destination
        if (!handleSrcDstCheck(sfs, spath, dfs, dpath)) return;
        
        // ensure parent directory exists
        if (!handleParentExists(dfs, dpath)) return;

        // handle overwrite logic
        bool overwritten;
        if (!handleOverwrite(dfs, dpath, overwrite, overwritten)) return;

        // do actual move
        bool ok = false;
        if (&sfs == &dfs)
            // atomic move on the same file system
            ok = sfs->rename(spath, dpath);
        else
        {
            // copy-delete aproach on different file systems
            if (!FileSystem::copyFileDir(sfs, spath, dfs, dpath))
                return m_server.sendCode(500, "Copy failed");
            ok = FileSystem::removeFileDir(sfs, spath);
        }

        // handle operation result
        if (!ok)
            return m_server.sendCode(500, "Move failed");
        if (overwritten)
            return m_server.sendCode(204, "Moved with overwrite");
        m_server.sendCode(201, "Moved");
    }

    void Handler::handleCOPY(FileSystem &sfs, const String &spath, FileSystem &dfs, const String &dpath, bool overwrite)
    {
        log_i("COPY: %s : %s -> %s : %s", 
            sfs.getName().c_str(), spath.c_str(), 
            dfs.getName().c_str(), dpath.c_str());
        
        // check source and destination
        if (!handleSrcDstCheck(sfs, spath, dfs, dpath)) return;

        // ensure parent directory exists
        if (!handleParentExists(dfs, dpath)) return;

        // handle overwrite logic
        bool overwritten;
        if (!handleOverwrite(dfs, dpath, overwrite, overwritten)) return;

        // do actual copy
        if (!FileSystem::copyFileDir(sfs, spath, dfs, dpath))
            return m_server.sendCode(500, "Copy failed");

        // handle operation result
        if (overwritten)
            return m_server.sendCode(204, "Copied with overwrite");
        m_server.sendCode(201, "Copied");
    }

    bool Handler::sendErrorCode(int code, const String &msg)
    {
        log_e("ERROR: %d : %s", code, msg.c_str());
        m_server.sendCode(code, msg);
        return false;
    }

    bool Handler::handleSrcDstCheck(FileSystem &sfs, const String &spath, FileSystem &dfs, const String &dpath)
    {
        if (!sfs->exists(spath))
            return sendErrorCode(404, "Source not found");
        if (&sfs == &dfs && spath == dpath)
            return sendErrorCode(403, "Source and destination are the same");
        if (&sfs == &dfs && dpath.startsWith(spath) && (dpath.length() == spath.length() || dpath[spath.length()] == '/'))
            return sendErrorCode(403, "Destination is subdirectory of source");
        return true;
    }

    bool Handler::handleParentExists(FileSystem &fs, const String &path)
    {
        String parent = path.substring(0, path.lastIndexOf('/'));
        if (!parent.isEmpty() && !fs->exists(parent))
            return sendErrorCode(409, "Parent directory does not exist");
        return true;
    }

    bool Handler::handleOverwrite(FileSystem &fs, const String &path, bool overwrite, bool &overwritten)
    {
        overwritten = false;
        if (fs->exists(path))
        {
            if (!overwrite)
                return sendErrorCode(412, "Overwrite forbidden");
            if (!FileSystem::removeFileDir(fs, path))
                return sendErrorCode(500, "Destination delete failed");
            overwritten = true;
        }
        return true;
    }
}
