#pragma once

#include <Arduino.h>
#include <FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/semphr.h>
#include <freertos/portmacro.h>

namespace core
{
    namespace TaskCpu
    {
        constexpr BaseType_t System      = 0;
        constexpr BaseType_t Application = 1;
    };

    namespace TaskPrio
    {
        constexpr UBaseType_t Background = 1;
        constexpr UBaseType_t Normal     = 2;
        constexpr UBaseType_t Realtime   = 3;
    };

    template<uint32_t stack, BaseType_t cpu, UBaseType_t prio>
    class Task
    {
        static inline portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;

    public:
        Task() noexcept = default;
        virtual ~Task() { stopRepeat(); stop(); }

        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        Task(Task&&) = delete;
        Task& operator=(Task&&) = delete;
    
    protected:
        bool start(const char* name)
        {
            return (!m_handle && xTaskCreatePinnedToCore(
                [](void* data) 
                {
                    auto instance = static_cast<Task*>(data);
                    instance->task();
                    instance->stop();
                },
                name, stack, this, prio, &m_handle, cpu
            ) == pdPASS);
        }

        void stop()
        {
            if (m_handle)
            {
                if (xTaskGetCurrentTaskHandle() == m_handle) 
                {
                    // stop by itself
                    m_handle = nullptr;
                    vTaskDelete(nullptr);
                } 
                else 
                {
                    // stop from outside
                    vTaskDelete(m_handle);
                    m_handle = nullptr;
                }
            }
        }

        bool startRepeat(const char* name, uint32_t periodMs)
        {
            bool ok = false;
            portENTER_CRITICAL(&s_mux);
            if (!m_timer && periodMs > 0)
            {
                if (m_timer = xTimerCreate(
                    name,
                    pdMS_TO_TICKS(periodMs),
                    pdTRUE,
                    this,
                    [](TimerHandle_t t) 
                    {
                        auto instance = static_cast<Task*>(pvTimerGetTimerID(t));
                        if (instance->isDead()) instance->start(pcTimerGetName(t));
                    }
                ))
                {
                    if (xTimerStart(m_timer, 0) == pdPASS) ok = true;
                    else { xTimerDelete(m_timer, 0); m_timer = nullptr; }
                }
            }
            portEXIT_CRITICAL(&s_mux);
            return ok;
        }

        void stopRepeat()
        {
            portENTER_CRITICAL(&s_mux);
            if (m_timer) 
            {
                xTimerStop(m_timer, 0);
                xTimerDelete(m_timer, 0);
                m_timer = nullptr;
            }
            portEXIT_CRITICAL(&s_mux);
        }

        void sleep(uint32_t periodMs) { vTaskDelay(pdMS_TO_TICKS(periodMs)); }
        bool isAlive() const { return m_handle != nullptr; }
        bool isDead() const { return m_handle == nullptr; }

        virtual void task() = 0;

    private:
        TaskHandle_t m_handle = nullptr;
        TimerHandle_t m_timer = nullptr;
    };

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

    class Timer final
    {
        time_t m_elapsedTS = 0;

    public:
        void start(time_t duration) { m_elapsedTS = millis() + duration; }
        void stop() { start(0); }

        bool elapsed() const { return millis() >= m_elapsedTS; }
        time_t remaining() const { return elapsed() ? 0 : m_elapsedTS - millis(); }    
    };
}
