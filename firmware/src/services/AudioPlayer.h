#pragma once
#ifndef NO_AUDIO

#include <AudioTools.h>
#include <AudioTools/AudioLibs/AudioRealFFT.h>
#include "core/tasks/Task.h"
#include "core/tasks/Mutex.h"
#include "AudioPlayer/AudioContext.h"
#include "AudioPlayer/AudioPlayerUI.h"
#include "AudioPlayer/FFTHandler.h"

namespace service
{
    class AudioPlayer
        : public task::Task<8192, task::core::Application, task::priority::Realtime>
    {
        static AudioPlayer* s_this;
        static void s_fftCallback(audio_tools::AudioFFTBase& fft);
        static void s_metadataCallback(audio_tools::MetaDataType type, const char* str, int len);

        enum class Command : uint8_t { Volume, Pause, Resume, Next, Prev, Index, Stop };

    public:
        void begin();
        bool start(details::AudioContext* context);
        void setVolume(float volume);
        void setPlaylistIndex(int index);
        void pause(bool yes);
        void next(bool fwd);
        void stop();

        bool isStarted();
        bool isPlaying();

        details::AudioPlayerUI& getUI() { return m_ui; }
        details::AudioContext*  getContext() { return m_context.get(); }
        void setFFTHandler(details::FFTHandler* fftHandler);

    private:
        void task() override;
        void fftCallback(audio_tools::AudioFFTBase& fft);
        void metadataCallback(audio_tools::MetaDataType type, const String& str);

    private:
        // this task access
        details::AudioPlayerUI m_ui;
        std::unique_ptr<details::AudioContext> m_context;
        audio_tools::LinearVolumeControl m_volCtr;
        audio_tools::MultiOutput m_output;
        audio_tools::AudioRealFFT m_fftOut;
        audio_tools::I2SStream m_i2sOut;

        // multi task access
        struct {
            task::Mutex mutex;
            audio_tools::AudioPlayer player;
            QueueHandle_t commands = nullptr;
            float param = 0;
        } m_play;
        struct {
            task::Mutex mutex;
            details::FFTHandler* handler = nullptr;
        } m_fft;
    };

    extern AudioPlayer audioPlayer;
}
#endif
