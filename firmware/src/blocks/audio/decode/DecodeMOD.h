#pragma once

#include "Decode.h"

namespace audio
{
    class DecodeMOD : public Decode
    {
    public:
        DecodeMOD();
        virtual ~DecodeMOD() override;

        bool begin(Source *source, Output *output) override;
        bool loop() override;
        bool stop() override;
        bool setSampleRate(int hz);
        bool setBufferSize(int sz);
        bool setStereoSeparation(int sep);
        bool setPAL(bool use);

    protected:
        bool LoadMOD();
        bool LoadHeader();
        void GetSample(int16_t sample[2]);
        bool RunPlayer();
        void LoadSamples();
        bool LoadPattern(uint8_t pattern);
        bool ProcessTick();
        bool ProcessRow();
        void Tremolo(uint8_t channel);
        void Portamento(uint8_t channel);
        void Vibrato(uint8_t channel);

    protected:
        int mixerTick;
        enum {BITDEPTH = 16};
        int sampleRate; 
        int fatBufferSize; //(6*1024) // File system buffers per-CHANNEL (i.e. total mem required is 4 * FATBUFFERSIZE)
        enum {FIXED_DIVIDER = 10};             // Fixed-point mantissa used for integer arithmetic
        int stereoSeparation; //STEREOSEPARATION = 32;    // 0 (max) to 64 (mono)
        bool usePAL;
        
        // Hz = 7093789 / (amigaPeriod * 2) for PAL
        // Hz = 7159091 / (amigaPeriod * 2) for NTSC
        int AMIGA;
        void UpdateAmiga() { AMIGA = ((usePAL?7159091:7093789) / 2 / sampleRate << FIXED_DIVIDER); }
    
        // support max 8 channels
        enum {ROWS = 64, SAMPLES = 31, CHANNELS = 8, NONOTE = 0xFFFF, NONOTE8 = 0xff };

        struct Sample 
        {
            uint16_t length;
            int8_t fineTune;
            uint8_t volume;
            uint16_t loopBegin;
            uint16_t loopLength;
        };
        
        struct mod 
        {
            Sample samples[SAMPLES];
            uint8_t songLength;
            uint8_t numberOfPatterns;
            uint8_t order[128];
            uint8_t numberOfChannels;
        };
        
        // Save 256 bytes by storing raw note values, unpack with macro NOTE
        struct Pattern 
        {
            uint8_t sampleNumber[ROWS][CHANNELS];
            uint8_t note8[ROWS][CHANNELS];
            uint8_t effectNumber[ROWS][CHANNELS];
            uint8_t effectParameter[ROWS][CHANNELS];
        };
        
        struct player 
        {
            Pattern currentPattern;
            
            uint32_t amiga;
            uint16_t samplesPerTick;
            uint8_t speed;
            uint8_t tick;
            uint8_t row;
            uint8_t lastRow;
            
            uint8_t orderIndex;
            uint8_t oldOrderIndex;
            uint8_t patternDelay;
            uint8_t patternLoopCount[CHANNELS];
            uint8_t patternLoopRow[CHANNELS];
            
            uint8_t lastSampleNumber[CHANNELS];
            int8_t volume[CHANNELS];
            uint16_t lastNote[CHANNELS];
            uint16_t amigaPeriod[CHANNELS];
            int16_t lastAmigaPeriod[CHANNELS];
            
            uint16_t portamentoNote[CHANNELS];
            uint8_t portamentoSpeed[CHANNELS];
            
            uint8_t waveControl[CHANNELS];
            
            uint8_t vibratoSpeed[CHANNELS];
            uint8_t vibratoDepth[CHANNELS];
            int8_t vibratoPos[CHANNELS];
            
            uint8_t tremoloSpeed[CHANNELS];
            uint8_t tremoloDepth[CHANNELS];
            int8_t tremoloPos[CHANNELS];
        };
        
        struct mixer 
        {
            uint32_t sampleBegin[SAMPLES];
            uint32_t sampleEnd[SAMPLES];
            uint32_t sampleloopBegin[SAMPLES];
            uint16_t sampleLoopLength[SAMPLES];
            uint32_t sampleLoopEnd[SAMPLES];
            
            uint8_t channelSampleNumber[CHANNELS];
            uint32_t channelSampleOffset[CHANNELS];
            uint16_t channelFrequency[CHANNELS];
            uint8_t channelVolume[CHANNELS];
            uint8_t channelPanning[CHANNELS];
        };
        
        struct fatBuffer 
        {
            uint8_t *channels[CHANNELS]; // Make dynamically allocated [FATBUFFERSIZE];
            uint32_t samplePointer[CHANNELS];
            uint8_t channelSampleNumber[CHANNELS];
        };

        // Effects
        enum EffectsValues 
        { 
            ARPEGGIO = 0, PORTAMENTOUP, PORTAMENTODOWN, TONEPORTAMENTO, VIBRATO, PORTAMENTOVOLUMESLIDE,
            VIBRATOVOLUMESLIDE, TREMOLO, SETCHANNELPANNING, SETSAMPLEOFFSET, VOLUMESLIDE, JUMPTOORDER,
            SETVOLUME, BREAKPATTERNTOROW, ESUBSET, SETSPEED 
        };
        
        // 0xE subset
        enum Effect08Subvalues 
        { 
            SETFILTER = 0, FINEPORTAMENTOUP, FINEPORTAMENTODOWN, GLISSANDOCONTROL, SETVIBRATOWAVEFORM,
            SETFINETUNE, PATTERNLOOP, SETTREMOLOWAVEFORM, SUBEFFECT8, RETRIGGERNOTE, FINEVOLUMESLIDEUP,
            FINEVOLUMESLIDEDOWN, NOTECUT, NOTEDELAY, PATTERNDELAY, INVERTLOOP 
        };
        
        // Our state lives here...
        player Player;
        mod Mod;
        mixer Mixer;
        fatBuffer FatBuffer;
    };
}
