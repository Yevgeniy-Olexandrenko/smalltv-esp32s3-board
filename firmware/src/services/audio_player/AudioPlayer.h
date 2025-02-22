#pragma once

#define USE_AUDIOTOOLS_NS false

#include <AudioTools.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/CodecAACHelix.h>
#include <AudioTools/AudioLibs/AudioRealFFT.h>
#include <FS.h>

#define USE_THREAD 1

namespace service
{
    class AudioPlayer
    {
    public:
        AudioPlayer();
        ~AudioPlayer();

        void begin();
        void loop();

    private:
        void task();
        void taskBegin();
        void taskLoop();

        void initSource();
        void initDecode();
        void initOutput();

        static void initStreamCallback();
        static Stream* nextStreamCallback(int offset);
        static void fftResultCallback(audio_tools::AudioFFTBase &fft);
        static void metadataCallback(audio_tools::MetaDataType type, const char* str, int len);

    private:
        audio_tools::I2SStream m_i2sOut;
        audio_tools::AudioRealFFT m_fftOut;
        audio_tools::MultiOutput m_output;
        audio_tools::AudioPlayer m_player;

        // audio context
        audio_tools::AudioSourceCallback* m_cbSrc = nullptr;
        audio_tools::MP3DecoderHelix* m_mp3Dec = nullptr;
        audio_tools::MetaDataFilterDecoder* m_id3Flt = nullptr;
        audio_tools::AudioSource* m_source = nullptr;
        audio_tools::AudioDecoder* m_decode = nullptr;
        fs::File m_dir;
        fs::File m_file;
    };

    extern AudioPlayer audioPlayer;
}
