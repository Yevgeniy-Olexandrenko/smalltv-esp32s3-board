#include <Esp.h>
#include "SelfReboot.h"
#include "drivers/graphics/Display.h"

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
