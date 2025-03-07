#pragma once

#include <FreeRTOS.h>

namespace task
{
    namespace priority
    {
        constexpr UBaseType_t Background = 1;
        constexpr UBaseType_t Normal     = 2;
        constexpr UBaseType_t Realtime   = 3;
    };

    namespace core
    {
        constexpr BaseType_t System      = 0;
        constexpr BaseType_t Application = 1;
    };
}