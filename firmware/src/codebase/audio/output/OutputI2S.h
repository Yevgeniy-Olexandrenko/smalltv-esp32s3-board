#pragma once

#include "Output.h"

namespace audio
{
    class OutputI2S : public Output
    {
    public:
        enum : int { APLL_AUTO = -1, APLL_ENABLE = 1, APLL_DISABLE = 0 };
        enum : int { EXTERNAL_I2S = 0, INTERNAL_DAC = 1, INTERNAL_PDM = 2 };

        OutputI2S(int port=0, int output_mode=EXTERNAL_I2S, int dma_buf_count = 8, int use_apll=APLL_DISABLE);
        ~OutputI2S() override;
    
        bool SetPinout(int bclkPin, int wclkPin, int doutPin);
        bool SetPinout(int bclkPin, int wclkPin, int doutPin, int mclkPin);
        
        bool SetRate(int hz) override;
        bool SetBitsPerSample(int bits) override;
        bool SetChannels(int channels) override;

        bool begin() override;
        bool ConsumeSample(int16_t sample[2]) override;
        void flush() override;
        bool stop() override;
        
        bool begin(bool txDAC);
        bool SetOutputModeMono(bool mono);  // Force mono output no matter the input
        bool SetLsbJustified(bool lsbJustified);  // Allow supporting non-I2S chips, e.g. PT8211 
        bool SetMclk(bool enabled);  // Enable MCLK output (if supported)
        bool SwapClocks(bool swap_clocks);  // Swap BCLK and WCLK

    protected:
        bool SetPinout();
        virtual int AdjustI2SRate(int hz) { return hz; }

    protected:
        uint8_t portNo;
        int output_mode;
        bool mono;
        int lsb_justified;
        bool i2sOn;
        int dma_buf_count;
        int use_apll;
        bool use_mclk;
        bool swap_clocks;

        // We can restore the old values and
        // free up these pins when in NoDAC mode
        uint32_t orig_bck;
        uint32_t orig_ws;
        
        uint8_t bclkPin;
        uint8_t wclkPin;
        uint8_t doutPin;
        uint8_t mclkPin;
    };
}