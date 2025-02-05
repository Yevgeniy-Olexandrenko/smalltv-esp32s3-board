#pragma once

#include "AudioType.h"
#include "shared/audio/source/Source.h"
#include "shared/audio/decode/Decode.h"
#include "shared/audio/output/Output.h"
#include "shared/audio/source/SourceExtractID3.h"

namespace service_audio_player_impl
{
    class AudioContext
    {
    public:
        AudioContext();
        virtual ~AudioContext();

        virtual bool open(const char* resource);
        virtual bool bind(audio::Output* output);
        virtual bool free();

        audio::Source* getSource() const { return _source; }
        audio::Decode* getDecode() const { return _decode; }

    protected:
        bool isFileNameEndsWithExt(const char* filepath, const char* ext) const;

    protected:
        audio::Source* _source;
        audio::Decode* _decode;
    };

    class MODFileAudioContext : public AudioContext
    {
    public:
        MODFileAudioContext();
        ~MODFileAudioContext() override;

        bool open(const char* resource) override;
    };

    class MP3FileAudioContext : public AudioContext
    {
    public:
        MP3FileAudioContext();
        ~MP3FileAudioContext() override;

        bool open(const char* resource) override;
        bool bind(audio::Output* output) override;

    private:
        audio::SourceExtractID3* _srcMP3;
    };

    AudioContext* createAudioContext(AudioType type);
    void destroyAudioContext(AudioContext* context);
}