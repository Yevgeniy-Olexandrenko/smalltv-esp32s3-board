#pragma once

#include "BackgroundService.h"

class WeatherServiceClass : public BackgroundService
{
public:
    void begin();
};

extern WeatherServiceClass WeatherService;
