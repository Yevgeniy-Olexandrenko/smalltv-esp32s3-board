#include <Esp.h>
#include "SelfReboot.h"

namespace driver
{
    void SelfReboot::reboot()
    {
        // TODO

        ESP.restart();
    }

    SelfReboot selfReboot;
}
