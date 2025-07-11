#pragma once

#include <qrcode.h>
#include <TFT_eSPI.h>

namespace image
{
    class QRCode
    {
    public:
        QRCode(uint8_t version);
        ~QRCode();

        void create(const char* str);
        void setColors(uint32_t bgColor, uint32_t fgColor);

        uint8_t  getSize() const;
        uint16_t getGfxSize(uint8_t scale) const;

        void renderOn(TFT_eSprite& sprite, uint8_t scale);
        void renderOn(TFT_eSPI& display, uint8_t scale, int32_t x, int32_t y);

    private:
        ::QRCode m_qrcode;
        uint32_t m_bgColor;
        uint32_t m_fgColor;
    };
}
