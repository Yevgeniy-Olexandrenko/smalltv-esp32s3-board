#include "WebDAVFS.h"

WebDAVFS::WebDAVFS(fs::FS &fs, const String &name, QuotaCb quotaCb)
    : m_fs(fs)
    , m_name(name)
    , m_quotaCb(quotaCb)
{
}

bool WebDAVFS::getQuota(QuotaSz &available, QuotaSz &used)
{
    available = 0, used = 0;
    if (m_quotaCb)
    {
        m_quotaCb(m_fs, available, used);
        return true;
    }
    return false;
}

String WebDAVFS::resolveURI(fs::File& file)
{
    String path = file.path();
    if (!path.startsWith("/")) path = "/" + path;
    if (file.isDirectory() && !path.endsWith("/")) path += "/";
    return (m_name + path);
}

String WebDAVFS::resolvePath(const String& decodedURI)
{
    String path = decodedURI.substring(m_name.length());
    if (path.isEmpty()) path = "/";
    if (path != "/" && path.endsWith("/"))
        path = path.substring(0, path.length() - 1);
    return path;
}

bool WebDAVFS::copyFileDir(WebDAVFS& sfs, const String& spath, WebDAVFS& dfs, const String& dpath)
{
    if (!sfs->exists(spath)) return false;

    // checks if the source is a file or directory
    if (fs::File file = sfs->open(spath, FILE_READ))
    {
        bool isDir = file.isDirectory();
        file.close();

        if (isDir)
            return copyDir(sfs, spath, dfs, dpath);
        else
            return copyFile(sfs, spath, dfs, dpath);
    }
    return false;
}

bool WebDAVFS::removeFileDir(WebDAVFS& fs, const String& path)
{
    if (!fs->exists(path)) return false;

    // checks if it's a file path and tries to delete it
    if (fs::File file = fs->open(path, FILE_READ))
    {
        if (!file.isDirectory()) return fs->remove(path);
    }
    else return false;

    // tries to delete a directory with its contents
    std::vector<String> dirs { path };
    for (size_t i = 0; i < dirs.size(); ++i)
    {
        if (fs::File dir = fs->open(dirs[i], FILE_READ))
        {
            if (dir.isDirectory())
            {
                bool isDir;
                String child;
                dir.rewindDirectory();
                while (!(child = dir.getNextFileName(&isDir)).isEmpty())
                {
                    if (isDir) dirs.push_back(child);
                    else if (!fs->remove(child)) return false;
                }
            }
        }
    }
    for (size_t i = dirs.size(); i-- > 0;) 
        if (!fs->rmdir(dirs[i])) return false;
    return true;
}

bool WebDAVFS::copyFile(WebDAVFS &sfs, const String &spath, WebDAVFS &dfs, const String &dpath)
{
    // open sourse and destination files
    fs::File sfile = sfs->open(spath, FILE_READ);
    if (!sfile) return false;
    fs::File dfile = dfs->open(dpath, FILE_WRITE);
    if (!dfile) return false;

    // allocate buffer in PSRAM if possible
    const size_t BUF_SIZE = 4096;
    uint8_t*  buf = (uint8_t*)ps_malloc(BUF_SIZE);
    if (!buf) buf = (uint8_t*)malloc(BUF_SIZE);
    if (!buf) return false;

    // copy file data, handle errors
    bool ok = true; int len;
    while ((len = sfile.read(buf, BUF_SIZE)) > 0) 
        if (dfile.write(buf, len) != len) { ok = false; break; }
    if (len < 0) ok = false;

    // free buffer and return result
    free(buf);
    return ok;
}

bool WebDAVFS::copyDir(WebDAVFS& sfs, const String& spath, WebDAVFS& dfs, const String& dpath)
{
    // create the root destination directory
    if (!dfs->mkdir(dpath)) return false;

    std::vector<String> sdirs { spath };
    std::vector<String> ddirs { dpath };
    for (size_t i = 0; i < sdirs.size(); ++i) 
    {
        // open the current source directory
        const String& sdir = sdirs[i];
        const String& ddir = ddirs[i];
        fs::File dir = sfs->open(sdir, FILE_READ);
        if (!dir || !dir.isDirectory()) return false;
        
        // process each entry in this directory
        dir.rewindDirectory();
        while (File entry = dir.openNextFile()) 
        {
            String schild = entry.path();
            String dchild = ddir + "/" + entry.name();
            if (entry.isDirectory()) 
            {
                // create the corresponding destination directory
                if (!dfs->mkdir(dchild)) return false;
                sdirs.push_back(schild);
                ddirs.push_back(dchild);
            }
            else
            {
                // copy individual file
                if (!copyFile(sfs, schild, dfs, dchild)) return false;
            }
        }
    }
    return true;
}
