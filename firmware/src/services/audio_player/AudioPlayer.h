#pragma once

#define USE_AUDIOTOOLS_NS false

#include <AudioTools.h>
#include "AudioContext.h"
#include "shared/settings/Settings.h"
#include "shared/tasks/Mutex.h"

namespace service
{
    using namespace service_audio_player_impl;

    class AudioPlayer : public settings::Provider
    {
        enum class Command : uint8_t { Volume, Pause, Resume, Next, Prev, Stop };

    public:
        void begin(float volume);
        bool start(AudioContext* context);
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
        void fetchFormats(String& output);
        void fetchPlaylists(const String& format, String& output);

        void task();
        static void fftResultCallback(audio_tools::AudioFFTBase& fft);
        static void metadataCallback(audio_tools::MetaDataType type, const char* str, int len);

    private:
        // player state in multitasking context
        struct {
            float volume = 0;
            TaskHandle_t handle = nullptr;
            AudioContext* context = nullptr;
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

        // app and webapp data
        struct {
            bool started = false;
            bool playing = false;
            uint8_t format = 0;
            uint8_t playlist = 0;
            String title;
            String artist;
        } m_ui;
    };

    extern AudioPlayer audioPlayer;
}
