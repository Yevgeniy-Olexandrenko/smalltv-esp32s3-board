#include "QRCode.h"

namespace image
{
    QRCode::QRCode(uint8_t version)
        : m_bgColor(TFT_WHITE)
        , m_fgColor(TFT_BLACK)
    {
        auto bufferSize  = qrcode_getBufferSize(version);
        m_qrcode.modules = new uint8_t[bufferSize];
        m_qrcode.version = version;
    }

    QRCode::~QRCode()
    {
        delete m_qrcode.modules;
    }

    void QRCode::create(const char *str)
    {
        qrcode_initText(&m_qrcode, m_qrcode.modules, m_qrcode.version, ECC_LOW, str);
    }

    void QRCode::setColors(uint32_t bgColor, uint32_t fgColor)
    {
        m_bgColor = bgColor;
        m_fgColor = fgColor;
    }

    uint8_t QRCode::getSize() const
    {
        return m_qrcode.size;
    }

    uint16_t QRCode::getGfxSize(uint8_t scale) const
    {
        return ((getSize() + 2) * scale);
    }

    void QRCode::renderOn(TFT_eSprite &sprite, uint8_t scale)
    {
        auto size = getSize();
        auto gfxSize = getGfxSize(scale);

        sprite.deleteSprite();
        sprite.createSprite(gfxSize, gfxSize);
        sprite.fillSprite(m_bgColor);

        for(uint8_t my = 0; my < size; ++my)
        {
            uint32_t sy = (my + 1) * scale;
            for(uint8_t mx = 0; mx < size; ++mx)
            {
                uint32_t sx = (mx + 1) * scale;
                uint32_t mc = (qrcode_getModule(&m_qrcode, mx, my) ? m_fgColor : m_bgColor);
                sprite.fillRect(sx, sy, scale, scale, mc);
            }
        }
    }

    void QRCode::renderOn(TFT_eSPI &display, uint8_t scale, int32_t x, int32_t y)
    {
        auto size = getSize();
        auto gfxSize = getGfxSize(scale);
        display.fillRect(x, y, gfxSize, gfxSize, m_bgColor);

        for(uint8_t my = 0; my < size; ++my)
        {
            uint32_t sy = (my + 1) * scale + y;
            for(uint8_t mx = 0; mx < size; ++mx)
            {
                uint32_t sx = (mx + 1) * scale + x;
                uint32_t mc = (qrcode_getModule(&m_qrcode, mx, my) ? m_fgColor : m_bgColor);
                display.fillRect(sx, sy, scale, scale, mc);
            }
        }
    }
}
