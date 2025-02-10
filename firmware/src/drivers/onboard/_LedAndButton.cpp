#include <Arduino.h>
#include "_LedAndButton.h"

namespace driver
{
    void LedAndButton::begin()
    {
        setLedMode(LedMode::Off);
        setLedBlinkPeriod();
        m_buttonState = false;
        m_buttonTime = 0;
    }

    void LedAndButton::update()
    {
        updateLedBlink();
        updateButtonState();
        setupLedState();
    }

    void LedAndButton::setLedMode(LedMode mode)
    {
        m_ledMode = mode;
        if (mode != LedMode::Blink)
            m_ledState = (mode == LedMode::On);
    }

    void LedAndButton::setLedBlinkPeriod(uint32_t onMs, uint32_t offMs)
    {
        m_ledOnPeriod = onMs;
        m_ledOffPeriod = offMs;
    }

    bool LedAndButton::getLedState() const
    {
        return m_ledState;
    }

    bool LedAndButton::getButtonState() const
    {
        return m_buttonState;
    }

    void LedAndButton::updateLedBlink() 
    {
        if (m_ledMode == LedMode::Blink) 
        {
            Timestamp now = millis();
            auto period = (m_ledState ? m_ledOnPeriod : m_ledOffPeriod);
            if (now - m_ledBlinkTime >= period) 
            {
                m_ledState = !m_ledState;
                m_ledBlinkTime = now;
            }
        }
    }

    void LedAndButton::updateButtonState()
    {
        Timestamp now = millis();
        if (now - m_buttonTime >= 1)
        {
            pinMode(ONBOARD_BUTTON_PIN, INPUT_PULLUP);
            m_buttonState = (digitalRead(ONBOARD_BUTTON_PIN) == LOW);
            m_buttonTime = now;
        }
    }

    void LedAndButton::setupLedState()
    {
        pinMode(ONBOARD_LED_PIN, OUTPUT);
        digitalWrite(ONBOARD_LED_PIN, m_ledState ? LOW : HIGH);
    }

    LedAndButton ledAndButton;
}
