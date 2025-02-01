#pragma once

#include <freertos/semphr.h>

class MutexLockGuard 
{
public:
    explicit MutexLockGuard(SemaphoreHandle_t mutex) 
        : _mutex(mutex) 
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
    }

    ~MutexLockGuard() 
    {
        xSemaphoreGive(_mutex);
    }

    MutexLockGuard(const MutexLockGuard&) = delete;
    MutexLockGuard& operator=(const MutexLockGuard&) = delete;

private:
    SemaphoreHandle_t _mutex;
};
