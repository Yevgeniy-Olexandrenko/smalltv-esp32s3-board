#pragma once

#include "WebDAVFS.h"
#include "WebDAVServer.h"

class WebDAVHandler
{
    class Resource
    {
    public:
        Resource(WebDAVServer& server, const String& uri, time_t modified);
        Resource(WebDAVServer& server, const String& uri, time_t modified, WebDAVFS::QuotaSz size);

        void setQuota(WebDAVFS::QuotaSz available, WebDAVFS::QuotaSz used);
        String toString() const;

    private:
        String reqProp(const String& prop, const String& val) const;
        String optProp(const String& prop, const String& val) const;

    private:
        String m_href, m_resourceType, m_lastModified;
        String m_contentLength, m_contentType, m_etag;
        String m_availableBytes, m_usedBytes; // RFC 4331
    };

public:
    void begin(const WebDAVServer& server);
    void addFS(fs::FS& fs, const String& mountName, WebDAVFS::QuotaCb quotaCb = nullptr);

    bool canHandle(HTTPMethod method, String uri);
    bool handle(HTTPMethod method, String uri);

    bool canRaw(HTTPMethod method, String uri);
    void raw(HTTPMethod method, String uri, HTTPRaw& raw);

private:
    bool canHandleURI(const String& uri);
    WebDAVFS* resolveFS(const String& uri);

    void handleOPTIONS  ();
    void handlePROPFIND (bool depth);
    void handlePROPFIND (WebDAVFS& fs, const String& path, bool depth);
    void handleMKCOL    (WebDAVFS& fs, const String& path);
    void handleDELETE   (WebDAVFS& fs, const String& path);
    void handleGET      (WebDAVFS& fs, const String& path, bool isHEAD);
    void handlePUT_init (WebDAVFS& fs, const String& path);
    void handlePUT_loop (HTTPRaw& raw, fs::File& file, bool overwrite);
    void handleMOVE     (WebDAVFS& sfs, const String& spath, WebDAVFS& dfs, const String& dpath, bool overwrite);
    void handleCOPY     (WebDAVFS& sfs, const String& spath, WebDAVFS& dfs, const String& dpath, bool overwrite);

    bool sendErrorCode(int code, const String& msg);
    void sendPROPFINDContent(std::vector<Resource> resources);
    
    bool handleSrcDstCheck(WebDAVFS& sfs, const String& spath, WebDAVFS& dfs, const String& dpath);
    bool handleParentExists(WebDAVFS& fs, const String& path);
    bool handleOverwrite(WebDAVFS& fs, const String& path, bool overwrite, bool& overwritten);

private:
    WebDAVServer m_server;
    std::vector<WebDAVFS> m_mountedFS;
    struct { fs::File file; bool overwrite = false; } m_upload;
};
