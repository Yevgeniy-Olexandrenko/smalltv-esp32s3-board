#pragma once

#include <FS.h>

class WebDAVFS
{
public:
    using QuotaSz = unsigned long long;
    using QuotaCb = std::function<void(fs::FS& fs, QuotaSz& available, QuotaSz& used)>;

    WebDAVFS(fs::FS& fs, const String& name, QuotaCb quotaCb);

    fs::FS* operator->() { return &m_fs; }
    const String& getName() const { return m_name; }
    bool getQuota(QuotaSz& available, QuotaSz& used);

    String resolveURI(fs::File& file);
    String resolvePath(const String& decodedURI);

    static bool copyFileDir(WebDAVFS &sfs, const String &spath, WebDAVFS &dfs, const String &dpath);
    static bool removeFileDir(WebDAVFS &fs, const String &path);

private:
    static bool copyFile(WebDAVFS& sfs, const String& spath, WebDAVFS& dfs, const String& dpath);
    static bool copyDir(WebDAVFS& sfs, const String& spath, WebDAVFS& dfs, const String& dpath);

private:
    fs::FS& m_fs;
    String  m_name;
    QuotaCb m_quotaCb;
};
