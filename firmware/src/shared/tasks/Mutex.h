#pragma once

#include <stdlib.h>
#include <freertos/semphr.h>

namespace task
{
    class Mutex final
    {
    public:
        // Create and destroy
        Mutex() : m_semaphore(xSemaphoreCreateBinary()) { xSemaphoreGive(m_semaphore); }
        ~Mutex() { vSemaphoreDelete(m_semaphore); }

        // Get and release lock
        void lock() { xSemaphoreTake(m_semaphore, portMAX_DELAY); }
        void unlock() { xSemaphoreGive(m_semaphore); }

        // Disable copying
        Mutex(const Mutex &) = delete;
        Mutex &operator=(const Mutex &) = delete;

        // Returns the underlying semaphore handle
        SemaphoreHandle_t getHandle() const { return m_semaphore; }

    private:
        SemaphoreHandle_t m_semaphore;
    };

    class LockGuard final
    {
    public:
        // Acquire and release the mutex
        explicit LockGuard(Mutex& mutex) : m_mutex(mutex) { m_mutex.lock(); }
        ~LockGuard() { m_mutex.unlock(); }

        // Disable copying
        LockGuard(const LockGuard &) = delete;
        LockGuard &operator=(const LockGuard &) = delete;

    private:
        Mutex& m_mutex;
    };
}
