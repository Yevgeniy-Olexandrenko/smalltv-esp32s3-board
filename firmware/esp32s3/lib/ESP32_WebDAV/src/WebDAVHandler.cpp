#include "WebDAVHandler.h"

WebDAVHandler::Resource::Resource(WebDAVServer& server, const String &uri, time_t modified)
    : m_href(server.encodeURI(uri))
    , m_lastModified(server.getHttpDateTime(modified))
{
    if (uri.endsWith("/"))
        m_resourceType = "<D:collection/>";
}

WebDAVHandler::Resource::Resource(WebDAVServer& server, const String &uri, time_t modified, WebDAVFS::QuotaSz size)
    : Resource(server, uri, modified)
{
    if (!uri.endsWith("/"))
    {
        m_contentLength = String(size);
        m_contentType = server.getContentType(uri);
    }
    m_etag = server.getETag(size, modified);
}

void WebDAVHandler::Resource::setQuota(WebDAVFS::QuotaSz available, WebDAVFS::QuotaSz used)
{
    m_availableBytes = String(available);
    m_usedBytes = String(used);
}

String WebDAVHandler::Resource::toString() const
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

String WebDAVHandler::Resource::reqProp(const String &prop, const String &val) const
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

String WebDAVHandler::Resource::optProp(const String &prop, const String &val) const
{
    if (val.isEmpty()) return "";
    return reqProp(prop, val);
}

////////////////////////////////////////////////////////////////////////////

void WebDAVHandler::begin(const WebDAVServer& server)
{
    m_server = server;
    m_server.attachHandler(*this);
    m_mountedFS.clear();
}

void WebDAVHandler::addFS(fs::FS& fs, const String& mountName, WebDAVFS::QuotaCb quotaCb)
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
        case HTTP_PROPPATCH:
            return true;

        case HTTP_MKCOL:
        case HTTP_DELETE:
        case HTTP_HEAD:
        case HTTP_GET:
        case HTTP_PUT:
        case HTTP_MOVE:
        case HTTP_COPY:
            return canHandleURI(uri);
    }
    return false;
}

bool WebDAVHandler::handle(HTTPMethod method, String uri)
{
    if (method == HTTP_OPTIONS)
    {
        handleOPTIONS();
        return true;
    }
    
    String decodedURI = m_server.decodeURI(uri);
    if (method == HTTP_PROPFIND || method == HTTP_PROPPATCH)
    {
        return handlePROPFIND(decodedURI);
    }

    if (WebDAVFS* sfs = resolveFS(decodedURI))
    {
        String spath = sfs->resolvePath(decodedURI);
        switch(method)
        {
            case HTTP_MKCOL:  handleMKCOL  (*sfs, spath   ); return true;
            case HTTP_DELETE: handleDELETE (*sfs, spath   ); return true;
            case HTTP_HEAD:   handleGET    (*sfs, spath, 1); return true;
            case HTTP_GET:    handleGET    (*sfs, spath, 0); return true;
        }

        // handle destination
        String hdrDestination = m_server.getHeader("Destination");
        if (!hdrDestination.isEmpty())
        {
            decodedURI = m_server.decodeURI(hdrDestination.substring(hdrDestination.indexOf('/', 8)));
            if (WebDAVFS* dfs = resolveFS(decodedURI))
            {
                // handle overwrite
                bool overwrite = true;
                String hdrOverwrite = m_server.getHeader("Overwrite");
                if (!hdrOverwrite.isEmpty()) 
                    overwrite = (hdrOverwrite[0] == 'T' || hdrOverwrite[0] == 't');

                // handle operation
                String dpath = dfs->resolvePath(decodedURI);
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

bool WebDAVHandler::canRaw(HTTPMethod method, String uri)
{
    return (method == HTTP_PUT ? canHandleURI(uri) : false);
}

void WebDAVHandler::raw(HTTPMethod method, String uri, HTTPRaw &raw)
{
    if (method == HTTP_PUT)
    {
        if (!m_upload.file)
        {
            if (raw.status == HTTPRawStatus::RAW_START)
            {
                String decodedURI = m_server.decodeURI(uri);
                if (WebDAVFS* fs = resolveFS(decodedURI))
                {
                    String path = fs->resolvePath(decodedURI);
                    handlePUT_init(*fs, path);
                }
            }
        }
        else
        {
            if (raw.status != HTTPRawStatus::RAW_START)
            {
                handlePUT_loop(raw, m_upload.file, m_upload.overwrite);
            }
        }
    }
}

bool WebDAVHandler::canHandleURI(const String &uri)
{
    String decodedURI = m_server.decodeURI(uri);
    return (resolveFS(decodedURI) != nullptr);
}

WebDAVFS *WebDAVHandler::resolveFS(const String &uri)
{
    for (auto& fs : m_mountedFS)
        if (uri.startsWith(fs.getName())) return &fs;
    return nullptr;
}

void WebDAVHandler::handleOPTIONS()
{
    log_i("OPTIONS");
    m_server.setHeader("DAV", "1");
    m_server.setHeader("Allow", "OPTIONS, PROPFIND, MKCOL, DELETE, HEAD, GET, PUT, MOVE, COPY");
    m_server.sendCode(200, "OK");
}

bool WebDAVHandler::handlePROPFIND(const String& decodedURI)
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
                    WebDAVFS::QuotaSz free, used;
                    if (fs.getQuota(free, used))
                    {
                        resources.emplace_back(
                            m_server, fs.resolveURI(childFile), fakeModifyTime, used);
                        resources.back().setQuota(free, used);
                    }
                    else
                    {
                        resources.emplace_back(
                            m_server, fs.resolveURI(childFile), fakeModifyTime);
                    }
                }
            }
        }
        resources.emplace_back(
            m_server, decodedURI, fakeModifyTime);
    }
    else
    {
        // check if request can be processed
        WebDAVFS* fs = resolveFS(decodedURI);
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
                if (childModifyTime > modifyTime) modifyTime = childModifyTime;
                resources.emplace_back(
                    m_server, fs->resolveURI(childFile), childModifyTime, childFile.size());
                childCount++;
            }
        }
        resources.emplace_back(
            m_server, fs->resolveURI(baseFile), modifyTime, childCount);
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
    {
        m_server.sendContent(resource.toString());
    }
    m_server.sendContent(xmlEpilogue);
    return true;
}

void WebDAVHandler::handleMKCOL(WebDAVFS& fs, const String& path)
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

void WebDAVHandler::handleDELETE(WebDAVFS& fs, const String& path)
{ 
    log_i("DELETE: %s : %s", fs.getName().c_str(), path.c_str());
    
    // do not remove the root
    if (path == "/")
        return m_server.sendCode(403, "Forbidden: can't delete root");

    // check if an object exists
    if (!fs->exists(path))
        return m_server.sendCode(404, "Not found");

    // trying to delete and check the result
    if (!WebDAVFS::removeFileDir(fs, path))
        return m_server.sendCode(500, "Delete failed");
    return m_server.sendCode(204, "Deleted");
}

void WebDAVHandler::handleGET(WebDAVFS& fs, const String& path, bool isHEAD)
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
    String etag = m_server.getETag(file.size(), file.getLastWrite());
    String lastmod = m_server.getHttpDateTime(file.getLastWrite());
    String contentType = m_server.getContentType(path);

    // handling caching and checking file for modification
    if (m_server.getHeader("If-None-Match") == etag ||
        m_server.getHeader("If-Modified-Since") == lastmod)
    {
        m_server.setHeader("ETag", etag);
        m_server.setHeader("Last-Modified", lastmod);
        return m_server.sendCode(304, "");
    }

    // setting cache parameters and sending the file
    m_server.setHeader("ETag", etag);
    m_server.setHeader("Last-Modified", lastmod);
    m_server.setHeader("Cache-Control", "private, max-age=0, must-revalidate");
    if (isHEAD)
    {
        m_server.setContentLength(file.size());
        m_server.sendCode(200, contentType, "");
    }
    else
        m_server.sendFile(file, contentType);
}

void WebDAVHandler::handlePUT_init(WebDAVFS &fs, const String &path)
{
    log_i("PUT: %s : %s", fs.getName().c_str(), path.c_str()); 
    
    // check that the distination is not a directory
    if (path.endsWith("/"))
        return m_server.sendCode(403, "Cannot PUT to directory");

    // ensure parent directory exists
    if (!handleParentExists(fs, path)) return;

    // try to open destination file
    bool exists = fs->exists(path);
    fs::File file = fs->open(path, FILE_WRITE);
    if (!file)
        return m_server.sendCode(500, "Failed to open file");

    m_upload.file = std::move(file);
    m_upload.overwrite = exists;
}

void WebDAVHandler::handlePUT_loop(HTTPRaw &raw, fs::File &file, bool overwrite)
{
    if (raw.status == HTTPRawStatus::RAW_WRITE)
    {
        // TODO
    }

    if (raw.status == HTTPRawStatus::RAW_END)
    {
        // TODO
    }

    if (raw.status == HTTPRawStatus::RAW_ABORTED)
    {
        // TODO
    }
}

void WebDAVHandler::handleMOVE(WebDAVFS &sfs, const String &spath, WebDAVFS &dfs, const String &dpath, bool overwrite)
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
        if (!WebDAVFS::copyFileDir(sfs, spath, dfs, dpath))
            return m_server.sendCode(500, "Copy failed");
        ok = WebDAVFS::removeFileDir(sfs, spath);
    }

    // handle operation result
    if (!ok)
        return m_server.sendCode(500, "Move failed");
    if (overwritten)
        return m_server.sendCode(204, "Moved with overwrite");
    m_server.sendCode(201, "Moved");
}

void WebDAVHandler::handleCOPY(WebDAVFS &sfs, const String &spath, WebDAVFS &dfs, const String &dpath, bool overwrite)
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
    if (!WebDAVFS::copyFileDir(sfs, spath, dfs, dpath))
        return m_server.sendCode(500, "Copy failed");

    // handle operation result
    if (overwritten)
        return m_server.sendCode(204, "Copied with overwrite");
    m_server.sendCode(201, "Copied");
}

bool WebDAVHandler::sendErrorCode(int code, const String &msg)
{
    m_server.sendCode(code, msg);
    return false;
}

bool WebDAVHandler::handleSrcDstCheck(WebDAVFS &sfs, const String &spath, WebDAVFS &dfs, const String &dpath)
{
    if (!sfs->exists(spath))
        return sendErrorCode(404, "Source not found");
    if (&sfs == &dfs && spath == dpath)
        return sendErrorCode(403, "Source and destination are the same");
    if (&sfs == &dfs && dpath.startsWith(spath) && (dpath.length() == spath.length() || dpath[spath.length()] == '/'))
        return sendErrorCode(403, "Destination is subdirectory of source");
    return true;
}

bool WebDAVHandler::handleParentExists(WebDAVFS &fs, const String &path)
{
    String parent = path.substring(0, path.lastIndexOf('/'));
    if (!parent.isEmpty() && !fs->exists(parent))
        return sendErrorCode(409, "Parent directory does not exist");
    return true;
}

bool WebDAVHandler::handleOverwrite(WebDAVFS &fs, const String &path, bool overwrite, bool &overwritten)
{
    overwritten = false;
    if (fs->exists(path))
    {
        if (!overwrite)
            return sendErrorCode(412, "Overwrite forbidden");
        if (!WebDAVFS::removeFileDir(fs, path))
            return sendErrorCode(500, "Destination delete failed");
        overwritten = true;
    }
    return true;
}
