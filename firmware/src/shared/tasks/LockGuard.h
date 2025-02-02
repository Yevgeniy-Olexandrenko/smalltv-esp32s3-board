#pragma once

#include <freertos/semphr.h>
#include "Mutex.h"

namespace task
{
    class LockGuard
    {
    public:
        // Acquires the mutex upon construction.
        explicit LockGuard(Mutex &mutex)
            : _mutexHandle(mutex.getHandle())
        {
            xSemaphoreTake(_mutexHandle, portMAX_DELAY);
        }

        // Releases the mutex upon destruction.
        ~LockGuard()
        {
            xSemaphoreGive(_mutexHandle);
        }

        // Disable copying
        LockGuard(const LockGuard &) = delete;
        LockGuard &operator=(const LockGuard &) = delete;

    private:
        SemaphoreHandle_t _mutexHandle;
    };
}
