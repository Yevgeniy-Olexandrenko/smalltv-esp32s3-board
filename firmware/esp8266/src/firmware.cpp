#include <Arduino.h>
#include "firmware.h"

void setup()
{
    Serial.begin(115200);
    for (int i = 0; i < 10 && !Serial; ++i) delay(500);

    drivers::begin();
    services::begin();
    apps::begin();
}

void loop()
{
    // do nothing
}
