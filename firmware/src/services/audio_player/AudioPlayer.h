#pragma once

#include <AudioTools.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>

namespace service
{
    class AudioPlayer
    {
    public:
        void begin();
        void loop();

    private:
        void task();
        void taskBegin();
        void taskLoop();
    };

    extern AudioPlayer audioPlayer;
}
