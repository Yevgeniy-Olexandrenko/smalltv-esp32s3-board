#pragma once

#define ONBOARD_LED_PIN    GPIO_NUM_0
#define ONBOARD_BUTTON_PIN GPIO_NUM_0

namespace driver
{
    class LedAndButton
    {
        using Timestamp = unsigned long;

    public:
        enum class LedMode { Off, On, Blink };

        void begin();
        void update();

        void setLedMode(LedMode mode);
        void setLedBlinkPeriod(uint32_t onMs = 500, uint32_t offMs = 500);

        bool getLedState() const;
        bool getButtonState() const;

    private:
        void updateLedBlink();
        void updateButtonState();
        void setupLedState();

    private:
        LedMode m_ledMode;
        uint32_t m_ledOnPeriod;
        uint32_t m_ledOffPeriod;

        Timestamp m_ledBlinkTime;
        Timestamp m_buttonTime;

        bool m_ledState;
        bool m_buttonState;
    };

    extern LedAndButton ledAndButton;
}
