#include "AudioContext.h"

#include "shared/audio/source/SourceFile.h"
#include "shared/audio/source/SourceMemory.h"

#include "shared/audio/decode/DecodeMOD.h"
#include "shared/audio/decode/DecodeMP3.h"

#include "drivers/storage/Storage.h"

namespace service_audio_player_impl
{
    ////////////////////////////////////////////////////////////////////////////

    AudioContext* createAudioContext(AudioType type)
    {
        switch(type)
        {
        case AudioType::MODFile: return new MODFileAudioContext();
        case AudioType::MP3File: return new MP3FileAudioContext();
        }
        return nullptr;
    }

    void destroyAudioContext(AudioContext *context)
    {
        if (context) delete context;
    }

    ////////////////////////////////////////////////////////////////////////////

    AudioContext::AudioContext()
        : _source(nullptr)
        , _decode(nullptr)
    {}

    AudioContext::~AudioContext()
    {
        free();
        if (_decode) delete _decode;
        if (_source) delete _source;
    }

    bool AudioContext::open(const char *resource)
    {
        return false;
    }

    bool AudioContext::bind(audio::Output *output)
    {
        if (_source && _decode && output && !_decode->isRunning())
        {
            return _decode->begin(_source, output);
        }
        return false;
    }

    bool AudioContext::free()
    {
        bool ok = true;
        if (_decode) ok &= _decode->stop();
        if (_source) ok &= _source->close();
        return ok;
    }

    bool AudioContext::isFileNameEndsWithExt(const char *filepath, const char *ext) const
    {
        if (filepath && ext)
        {
            // prepare strings for parsing
            auto strExt = String(ext), strPath = String(filepath);
            auto strName = strPath.substring(strPath.lastIndexOf('/'));

            // extension and file name must not be empty
            if (strExt.isEmpty() || strName.isEmpty()) return false;

            // ignore character case
            strExt.toLowerCase();
            strName.toLowerCase();

            // hidden files not allowed
            if (strName.charAt(0) == '.') return false;
            return strName.endsWith('.' + strExt);
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////

    MODFileAudioContext::MODFileAudioContext()
    {
        _source = new audio::SourceMemory();
        _decode = new audio::DecodeMOD();
    }

    MODFileAudioContext::~MODFileAudioContext()
    {}

    bool MODFileAudioContext::open(const char *resource)
    {
        if (_source && isFileNameEndsWithExt(resource, "mod"))
        {
            auto source = static_cast<audio::SourceMemory*>(_source);
            return source->open(driver::storage.getFS(), resource);
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    MP3FileAudioContext::MP3FileAudioContext()
    {
        _source = new audio::SourceFile();
        _srcMP3 = new audio::SourceExtractID3(_source);
        _decode = new audio::DecodeMP3();
    }

    MP3FileAudioContext::~MP3FileAudioContext()
    {
        delete _srcMP3;
    }

    bool MP3FileAudioContext::open(const char *resource)
    {
        if (_source && isFileNameEndsWithExt(resource, "mp3"))
        {
            auto source = static_cast<audio::SourceFile*>(_source);
            return source->open(driver::storage.getFS(), resource);
        }
    }

    bool MP3FileAudioContext::bind(audio::Output *output)
    {
        if (_srcMP3 && _decode && output && !_decode->isRunning())
        {
            return _decode->begin(_srcMP3, output);
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
}
