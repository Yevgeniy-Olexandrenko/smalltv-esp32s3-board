#include <FS.h>
#include "WebDAV.h"

namespace core
{
    void WebDAV::begin(WebServer& server, fs::FS& fs, const String& mountPoint)
    {
        m_fs = &fs;
        m_mp = mountPoint;
        server.addHandler(this);
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
        switch(method)
        {
            case HTTP_OPTIONS:  handleOPTIONS(server);  break;
            case HTTP_GET:      handleGET(server);      break;
            case HTTP_PUT:      handlePUT(server);      break;
            case HTTP_DELETE:   handleDELETE(server);   break;
            case HTTP_COPY:     handleCOPY(server);     break;
            case HTTP_MKCOL:    handleMKCOL(server);    break;
            case HTTP_MOVE:     handleMOVE(server);     break;
            case HTTP_PROPFIND: handlePROPFIND(server); break;
            default:
                return false;
        }
        return true;
    }

    void WebDAV::handleOPTIONS(WebServer& server)
    {
        log_i("handleOPTIONS");

        server.sendHeader("DAV",   "1,2");
        server.sendHeader("Allow", "OPTIONS, GET, PROPFIND, PUT, DELETE, MKCOL, COPY, MOVE");
        server.send(200);
    }

    void WebDAV::handlePROPFIND(WebServer& server)
    {
        log_i("handlePROPFIND");

        String uri = server.uri();
        if (!uri.endsWith("/")) uri += "/";

        String body = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<D:multistatus xmlns:D=\"DAV:\">";
        body += "<D:response>";
        body +=   "<D:href>" + uri + "</D:href>";
        body +=   "<D:propstat>";
        body +=     "<D:prop>";
        body +=       "<D:resourcetype><D:collection/></D:resourcetype>";
        body +=       "<D:displayname>" + uri.substring(m_mp.length()) + "</D:displayname>";
        body +=       "<D:getlastmodified>Fri, 01 Aug 2025 12:00:00 GMT</D:getlastmodified>";
        body +=     "</D:prop>";
        body +=     "<D:status>HTTP/1.1 200 OK</D:status>";
        body +=   "</D:propstat>";
        body += "</D:response>";
        body += "</D:multistatus>";

        server.sendHeader("Content-Type", "application/xml; charset=\"utf-8\"");
        server.sendHeader("Content-Length", String(body.length()));
        server.send(207, "application/xml", body);
    }

    void WebDAV::handleGET(WebServer& server)
    {
        log_i("handleGET");

        String relPath = server.uri().substring(m_mp.length());
        fs::File f = m_fs->open(relPath, "r");
        if (!f) {
            server.send(404);
            return;
        }
        
        server.streamFile(f, "application/octet-stream");
        f.close();
    }

    void WebDAV::handlePUT(WebServer& server)    { log_i("handlePUT"   ); server.send(501); }
    void WebDAV::handleDELETE(WebServer& server) { log_i("handleDELETE"); server.send(501); }
    void WebDAV::handleMKCOL(WebServer& server)  { log_i("handleMKCOL" ); server.send(501); }
    void WebDAV::handleCOPY(WebServer& server)   { log_i("handleCOPY"  ); server.send(501); }
    void WebDAV::handleMOVE(WebServer& server)   { log_i("handleMOVE"  ); server.send(501); }

}
