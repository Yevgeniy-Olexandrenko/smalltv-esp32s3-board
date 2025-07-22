#pragma once

#include <Arduino.h>

namespace core
{
    int h2i(char c)
    {
        c = tolower(c);
        return 
            c >= '0' && c <= '9' ? c - '0' :
            c >= 'a' && c <= 'f' ? c - 'a' + 10 :
            -1;
    }

    char i2h(int i)
    {
        return i <= 9 ? i + '0' : i - 10 + 'A';
    }

    int hh2i(const char* c)
    {
        int h = h2i(*(c + 0));
        int l = h2i(*(c + 1));
        return h < 0 || l < 0 ? -1 : (h << 4) + l;
    }

    bool notEncodable (char c)
    {
        return c > 32 && c < 127;
    }

    String c2enc(const String &decoded) {
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

    String enc2c(const String &encoded) {
        String decoded;
        decoded.reserve(encoded.length());
        for (size_t i = 0; i < encoded.length(); ++i) {
            char c = encoded[i];
            if (c == '%' && i + 2 < encoded.length()) {
                char hi = encoded[i+1];
                char lo = encoded[i+2];
                uint8_t val = 0;
                if      (hi >= '0' && hi <= '9') val = (hi - '0') << 4;
                else if (hi >= 'A' && hi <= 'F') val = (hi - 'A' + 10) << 4;
                else if (hi >= 'a' && hi <= 'f') val = (hi - 'a' + 10) << 4;
                if      (lo >= '0' && lo <= '9') val |= (lo - '0');
                else if (lo >= 'A' && lo <= 'F') val |= (lo - 'A' + 10);
                else if (lo >= 'a' && lo <= 'f') val |= (lo - 'a' + 10);
                decoded += char(val);
                i += 2;
            } else {
                decoded += c;
            }
        }
        return decoded;
    }


}
