#pragma once

#include <FS.h>
#include "AudioContext.h"
#include "shared/tasks/Thread.h"

namespace service
{
    using namespace service_audio_player_impl;

    class AudioPlayer
    {
    public:
        AudioPlayer();
        ~AudioPlayer();

        bool begin(AudioType type, fs::File& dir);
        void pause();
        void play();
        void end();

        uint8_t getVolume() const;
        void setVolume(uint8_t volume);
        bool isPlaying() const;

    private:
        AudioType _type;
        uint8_t _volume;

        fs::File _dir;

        audio::Output* _output;
        AudioContext*  _context;
        task::Thread*  _thread;

        
    };
}