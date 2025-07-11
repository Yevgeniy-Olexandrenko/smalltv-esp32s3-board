#include <Esp.h>
#include "SelfReboot.h"
#include "Display.h"

namespace driver
{
    void SelfReboot::reboot()
    {
        // TODO

        #ifndef NO_VIDEO
        display.fadeOut();
        #endif
        ESP.restart();
    }

    SelfReboot selfReboot;
}
