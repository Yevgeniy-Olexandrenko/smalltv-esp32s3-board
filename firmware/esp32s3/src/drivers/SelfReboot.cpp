#include <Esp.h>
#include "hardware/board.h"
#include "SelfReboot.h"
#include "Display.h"

namespace driver
{
    void SelfReboot::begin()
    {
        #ifndef NO_SELFRES
        pinMode(PIN_ESP_RES, OUTPUT);
        digitalWrite(PIN_ESP_RES, LOW);
        #endif
    }

    void SelfReboot::reboot()
    {
        // fade out display
        #ifndef NO_VIDEO
        display.fadeOut();
        #endif

        // do actual reset
        #ifndef NO_SELFRES
        digitalWrite(PIN_ESP_RES, HIGH);
        delay(1000);
        #else
        ESP.restart();
        #endif
    }

    SelfReboot selfReboot;
}
