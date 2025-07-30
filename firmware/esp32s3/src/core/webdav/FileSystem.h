#pragma once

#include <FS.h>

namespace WebDAV
{
    class FileSystem
    {
    public:
        using QuotaSz = unsigned long long;
        using QuotaCb = std::function<void(fs::FS& fs, QuotaSz& available, QuotaSz& used)>;

        FileSystem(fs::FS& fs, const String& name, QuotaCb quotaCb)
            : m_fs(fs)
            , m_name(name)
            , m_quotaCb(quotaCb)
        {}

        fs::FS* operator->() { return &m_fs; }
        const String& getName() const { return m_name; }
        bool getQuota(QuotaSz& available, QuotaSz& used);

        String resolveURI(fs::File& file);
        String resolvePath(const String& decodedURI);

        static bool copyFileDir(FileSystem &sfs, const String &spath, FileSystem &dfs, const String &dpath);
        static bool removeFileDir(FileSystem &fs, const String &path);

    private:
        static bool copyFile(FileSystem& sfs, const String& spath, FileSystem& dfs, const String& dpath);
        static bool copyDir(FileSystem& sfs, const String& spath, FileSystem& dfs, const String& dpath);

    private:
        fs::FS& m_fs;
        String  m_name;
        QuotaCb m_quotaCb;
    };
}
