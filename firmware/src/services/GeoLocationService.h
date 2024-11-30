#pragma once

#include "BackgroundService.h"

class GeoLocationServiceClass : public BackgroundService
{
public:
    void begin();
};

extern GeoLocationServiceClass GeoLocationService;
