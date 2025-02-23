#pragma once

namespace service
{
    struct AudioContent
    {
        enum class Format { mp3, acc, wav };
        enum class Location { StorageFile, RadioStream };

        Format format;
        Location location;
        const char* extOrContentType;

    protected:
        AudioContent(Format format, Location location, const char* extOrContentType)
            : format(format), location(location), extOrContentType(extOrContentType)
        {}
    };

    struct StorageFileMP3 : public AudioContent
    {
        StorageFileMP3()
            : AudioContent(Format::mp3, Location::StorageFile, "mp3")
        {}
    };

    struct StorageFileACC : public AudioContent
    {
        StorageFileACC()
            : AudioContent(Format::acc, Location::StorageFile, "acc")
        {}
    };

    struct StorageFileWAV : public AudioContent
    {
        StorageFileWAV()
            : AudioContent(Format::wav, Location::StorageFile, "wav")
        {}
    };

    struct RadioStreamMP3 : public AudioContent
    {
        RadioStreamMP3()
            : AudioContent(Format::mp3, Location::RadioStream, "audio/mp3")
        {}
    };

    struct RadioStreamACC : public AudioContent
    {
        RadioStreamACC()
            : AudioContent(Format::acc, Location::RadioStream, "audio/acc")
        {}
    };
}