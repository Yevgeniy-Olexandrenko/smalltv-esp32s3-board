#pragma once

#define USE_AUDIOTOOLS_NS false

#include <AudioTools.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/CodecAACHelix.h>
#include <AudioTools/AudioLibs/AudioRealFFT.h>
#include <FS.h>

#include "AudioContent.h"
#include "shared/settings/Settings.h"
#include "shared/tasks/Mutex.h"

namespace service
{
    class AudioPlayer : public settings::Provider
    {
        enum class Command : uint8_t { Volume, Pause, Resume, Next, Prev, Stop };

    public:
        void begin(float volume);
        bool start(AudioContent content, const char* resource);
        void setVolume(float volume);
        void pause(bool yes);
        void next(bool fwd);
        void stop();

        bool isStarted();
        bool isPlaying();

    public:
        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        void task();

        void initSource();
        void initDecode();

        void deinitSource();
        void deinitDecode();

        // audio context
        static void initStreamCallback();
        static Stream* nextStreamCallback(int offset);

        // player callbacks
        static void fftResultCallback(audio_tools::AudioFFTBase& fft);
        static void metadataCallback(audio_tools::MetaDataType type, const char* str, int len);

    private:
        // player state in multitasking context
        struct {
            float volume = 0;
            TaskHandle_t handle = nullptr;
            audio_tools::AudioPlayer player;
        } m_task;
        QueueHandle_t m_cmdQueue;
        task::Mutex m_mutex;

        // output components
        audio_tools::I2SStream m_i2sOut;
        audio_tools::AudioRealFFT m_fftOut;
        audio_tools::MultiOutput m_output;

        // volume constrol component
        audio_tools::LinearVolumeControl m_volCtr;

        // audio context
        audio_tools::AudioSourceCallback* m_cbSrc = nullptr;
        audio_tools::MP3DecoderHelix* m_mp3Dec = nullptr;
        audio_tools::MetaDataFilterDecoder* m_id3Flt = nullptr;
        audio_tools::AudioSource* m_source = nullptr;
        audio_tools::AudioDecoder* m_decode = nullptr;
        String m_path;
        fs::File m_dir;
        fs::File m_file;
    };

    extern AudioPlayer audioPlayer;
}
