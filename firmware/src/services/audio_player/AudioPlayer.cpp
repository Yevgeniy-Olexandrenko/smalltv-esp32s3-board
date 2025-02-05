#include "AudioPlayer.h"
#include "shared/audio/output/OutputI2S.h"
#include "board.h"

namespace service
{
    AudioPlayer::AudioPlayer()
        : _type(AudioType::Unknown)
        , _volume(100)
    {
    }

    AudioPlayer::~AudioPlayer()
    {
    }

    bool AudioPlayer::begin(AudioType type, fs::File& dir)
    {
        // stop player and free resouces
        end();

        // acquire resources and start player
        if (type != AudioType::Unknown && dir.isDirectory())
        {
            // create and prepare audio output
            auto output = new audio::OutputI2S();
            if (output->SetPinout(PIN_SND_BCLK, PIN_SND_RLCLK, PIN_SND_DIN))
            {
                auto gain = float(_volume) / 100.f;
                output->SetRate(44100);
                output->SetBitsPerSample(16);
                output->SetChannels(2);
                output->SetGain(gain);
            } 
            else
            {
                delete output;
                return false;
            }

            // create and prepare audio context
            auto context = createAudioContext(type);
            if (!context) return false;

            // ok, everything looks fine
            _context = context;
            _output = output;
            _type = type;
            _dir = dir;

            // start to play the first file
            // TODO

            return true;
        }
        return false;
    }

    void AudioPlayer::pause()
    {
    }

    void AudioPlayer::play()
    {
    }

    void AudioPlayer::end()
    {
    }

    uint8_t AudioPlayer::getVolume() const
    {
        return _volume;
    }

    void AudioPlayer::setVolume(uint8_t volume)
    {
        if (volume <= 200)
        {
            if (_output)
            {
                auto gain = (float(volume) / 100.f);
                _output->SetGain(gain);
            }
            _volume = volume;
        }
    }

    bool AudioPlayer::isPlaying() const
    {
        return false;
    }
}
