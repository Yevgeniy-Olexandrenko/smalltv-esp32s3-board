#pragma once

#include <stdlib.h>
#include <freertos/semphr.h>

namespace task
{
    class Mutex
    {
    public:
        Mutex()
        {
            _handle = xSemaphoreCreateMutex();
            if (!_handle) ::abort();
        }

        ~Mutex()
        {
            if (_handle)
                vSemaphoreDelete(_handle);
        }

        // Disable copying
        Mutex(const Mutex &) = delete;
        Mutex &operator=(const Mutex &) = delete;

        // Returns the underlying semaphore handle
        SemaphoreHandle_t getHandle() const { return _handle; }

    private:
        SemaphoreHandle_t _handle;
    };
}
