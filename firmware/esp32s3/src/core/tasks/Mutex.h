#pragma once

#include <stdlib.h>
#include <FreeRTOS.h>
#include <freertos/semphr.h>

namespace task
{
    class Mutex final
    {
    public:
        // create and destroy
        Mutex() : m_semaphore(xSemaphoreCreateMutex()) {}
        ~Mutex() { vSemaphoreDelete(m_semaphore); }

        // get and release lock
        void lock() { xSemaphoreTake(m_semaphore, portMAX_DELAY); }
        void unlock() { xSemaphoreGive(m_semaphore); }

        // disable copying
        Mutex(const Mutex &) = delete;
        Mutex &operator=(const Mutex &) = delete;

        // returns the underlying semaphore handle
        SemaphoreHandle_t getHandle() const { return m_semaphore; }

    private:
        SemaphoreHandle_t m_semaphore;
    };

    class LockGuard final
    {
    public:
        // acquire and release the mutex
        explicit LockGuard(Mutex& mutex) : m_mutex(mutex) { m_mutex.lock(); }
        ~LockGuard() { m_mutex.unlock(); }

        // disable copying
        LockGuard(const LockGuard &) = delete;
        LockGuard &operator=(const LockGuard &) = delete;

    private:
        Mutex& m_mutex;
    };
}
