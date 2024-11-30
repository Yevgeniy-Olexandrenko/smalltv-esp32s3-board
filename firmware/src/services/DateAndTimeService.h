#pragma once

#include "BackgroundService.h"

class DateAndTimeServiceClass : public BackgroundService
{
public:
    void begin();
};

extern DateAndTimeServiceClass DateAndTimeService;
