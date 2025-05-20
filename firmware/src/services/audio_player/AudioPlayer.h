#pragma once
#ifndef NO_AUDIO

#include <AudioTools.h>
#include <AudioTools/AudioLibs/AudioRealFFT.h>
#include "core/tasks/Task.h"
#include "core/tasks/Mutex.h"
#include "core/audio/FFTHandler.h"
#include "AudioContext.h"
#include "AudioPlayerUI.h"

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
        bool start(audio_player::AudioContext* context);
        void setVolume(float volume);
        void setPlaylistIndex(int index);
        void pause(bool yes);
        void next(bool fwd);
        void stop();

        bool isStarted();
        bool isPlaying();

        audio_player::AudioPlayerUI& getUI() { return m_ui; }
        audio_player::AudioContext*  getContext() { return m_context.get(); }
        void setFFTHandler(audio::FFTHandler* fftHandler);

    private:
        void task() override;
        void fftCallback(audio_tools::AudioFFTBase& fft);
        void metadataCallback(audio_tools::MetaDataType type, const String& str);

    private:
        // this task access
        audio_player::AudioPlayerUI m_ui;
        std::unique_ptr<audio_player::AudioContext> m_context;
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
            audio::FFTHandler* handler = nullptr;
        } m_fft;
    };

    extern AudioPlayer audioPlayer;
}
#endif
