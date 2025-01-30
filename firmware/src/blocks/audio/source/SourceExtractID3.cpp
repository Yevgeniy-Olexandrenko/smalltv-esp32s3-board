#include <string.h>
#include "SourceExtractID3.h"

namespace audio
{
    // Handle unsync operation in ID3 with custom class
    class SourceUnsync : public Source
    {
    public:
        SourceUnsync(Source* src, int len, bool unsync)
            : _source(src)
            , _remaining(len)
            , _isUnsync(unsync)
            , _savedByte(-1)
        {}

        ~SourceUnsync() override {}
        
        uint32_t read(void *data, uint32_t len) override
        {
            uint32_t bytes = 0;
            uint8_t *ptr = reinterpret_cast<uint8_t *>(data);

            // This is only used during ID3 parsing, so no need to optimize here...
            while (len--)
            {
                int b = getByte();
                if (b >= 0)
                {
                    *(ptr++) = (uint8_t)b;
                    bytes++;
                }
            }
            return bytes;
        }

        int getByte()
        {
            // If we're not unsync, just read.
            if (!_isUnsync)
            {
                uint8_t c;
                if (!_remaining)
                    return -1;
                _remaining--;
                if (1 != _source->read(&c, 1))
                    return -1;
                return c;
            }

            // If we've saved a pre-read character, return it immediately
            if (_savedByte >= 0)
            {
                int s = _savedByte;
                _savedByte = -1;
                return s;
            }

            if (_remaining <= 0)
            {
                return -1;
            }
            else if (_remaining == 1)
            {
                _remaining--;
                uint8_t c;
                if (1 != _source->read(&c, 1))
                    return -1;
                else
                    return c;
            }
            else
            {
                uint8_t c;
                _remaining--;
                if (1 != _source->read(&c, 1))
                    return -1;
                if (c != 0xff)
                {
                    return c;
                }
                // Saw 0xff, check next byte.  If 0 then eat it, OTW return the 0xff
                uint8_t d;
                _remaining--;
                if (1 != _source->read(&d, 1))
                    return c;
                if (d != 0x00)
                {
                    _savedByte = d;
                }
                return c;
            }
        }

        bool eof()
        {
            return (_remaining <= 0);
        }

    private:
        Source* _source;
        bool _isUnsync;
        int _remaining;
        int _savedByte;
    };

    uint32_t SourceExtractID3::read(void *data, uint32_t len)
    {
        if (_checked)
        {
            return _source->read(data, len);
        }

        _checked = true;

        // <10 bytes initial read, not enough space to check header
        if (len < 10)
            return _source->read(data, len);

        uint8_t *buff = reinterpret_cast<uint8_t *>(data);
        int ret = _source->read(data, 10);
        if (ret < 10) return ret;

        if (buff[0] != 'I' || buff[1] != 'D' || buff[2] != '3' || buff[3] > 0x04 || buff[3] < 0x02 || buff[4] != 0)
        {
            _metadata("eof", false, "id3");
            return (10 + _source->read(buff + 10, len - 10));
        }

        int rev = buff[3];
        bool unsync = false;
        bool exthdr = false;

        switch (rev)
        {
        case 2:
            unsync = (buff[5] & 0x80);
            exthdr = false;
            break;
        case 3:
        case 4:
            unsync = (buff[5] & 0x80);
            exthdr = (buff[5] & 0x40);
            break;
        };

        int id3Size = buff[6];
        id3Size = id3Size << 7;
        id3Size |= buff[7];
        id3Size = id3Size << 7;
        id3Size |= buff[8];
        id3Size = id3Size << 7;
        id3Size |= buff[9];

        // Every read from now may be unsync'd
        SourceUnsync id3(_source, id3Size, unsync);

        if (exthdr)
        {
            int ehsz = (id3.getByte() << 24) | (id3.getByte() << 16) | (id3.getByte() << 8) | (id3.getByte());
            for (int j = 0; j < ehsz - 4; j++) id3.getByte(); // Throw it away
        }

        do
        {
            unsigned char frameid[4];
            int framesize;
            bool compressed;

            frameid[0] = id3.getByte();
            frameid[1] = id3.getByte();
            frameid[2] = id3.getByte();
            if (rev == 2)
                frameid[3] = 0;
            else
                frameid[3] = id3.getByte();

            if (frameid[0] == 0 && frameid[1] == 0 && frameid[2] == 0 && frameid[3] == 0)
            {
                // We're in padding
                while (!id3.eof())
                {
                    id3.getByte();
                }
            }
            else
            {
                if (rev == 2)
                {
                    framesize = (id3.getByte() << 16) | (id3.getByte() << 8) | (id3.getByte());
                    compressed = false;
                }
                else
                {
                    framesize = (id3.getByte() << 24) | (id3.getByte() << 16) | (id3.getByte() << 8) | (id3.getByte());
                    id3.getByte(); // skip 1st flag
                    compressed = id3.getByte() & 0x80;
                }
                if (compressed)
                {
                    int decompsize = (id3.getByte() << 24) | (id3.getByte() << 16) | (id3.getByte() << 8) | (id3.getByte());
                    // TODO - add libz decompression, for now ignore this one...
                    (void)decompsize;
                    for (int j = 0; j < framesize; j++)
                        id3.getByte();
                }

                // Read the value and send to callback
                char value[64];
                uint32_t i;
                bool isUnicode = (id3.getByte() == 1) ? true : false;
                for (i = 0; i < (uint32_t)framesize - 1; i++)
                {
                    if (i < sizeof(value) - 1)
                        value[i] = id3.getByte();
                    else
                        (void)id3.getByte();
                }
                value[i < sizeof(value) - 1 ? i : sizeof(value) - 1] = 0; // Terminate the string...

                auto isFrame = [&](const char* r3id, const char* r2id)
                {
                    bool _r3id = (frameid[0] == r3id[0] && frameid[1] == r3id[1] && frameid[2] == r3id[2] && frameid[3] == r3id[3]);
                    bool _r2id = (r2id && frameid[0] == r2id[0] && frameid[1] == r2id[1] && frameid[2] == r2id[2] && frameid[3] == 0);
                    return (_r3id || _r2id);
                };
                auto md = [&](const char* name) { _metadata(name, isUnicode, value); };

                if (isFrame("TIT2", "TT2")) md("title");
                else if (isFrame("TPE1", "TP1")) md("performer");
                else if (isFrame("TPE2", "TP2")) md("band");
                else if (isFrame("TALB", "TAL")) md("album");
                else if (isFrame("TYER", "TYE")) md("year");
                else if (isFrame("TRCK", "TRK")) md("track");
                else if (isFrame("TPOS", "TPA")) md("part_of_set");
                else if (isFrame("POPM", "POP")) md("popularimeter");
                else if (isFrame("TCOM", "TCM")) md("composer");
                else if (isFrame("TCON", "TCO")) md("content_type");
                else if (isFrame("TDRC", nullptr)) md("recording_time");
                else
                {
                    char name[] = { frameid[0], frameid[1], frameid[2], frameid[3], 0 };
                    _metadata(name, isUnicode, value);
                }
            }
        } while (!id3.eof());

        // use callback function to signal end of tags and beginning of content.
        _metadata("eof", false, "id3");

        // All ID3 processing done, return to main caller
        return _source->read(data, len);
    }
}
