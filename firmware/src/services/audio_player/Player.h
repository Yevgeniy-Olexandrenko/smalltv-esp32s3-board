#pragma once

namespace service
{
    class Player
    {
    public:
        void begin();
        void pause();
        void play();
        void end();

        bool isPlaying() const;

    private:

    };

    extern Player audioPlayer;
}
