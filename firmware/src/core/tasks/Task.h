#pragma once

#include <FreeRTOS.h>
#include <freertos/task.h>

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

    template<uint32_t stack, BaseType_t core, UBaseType_t priority>
    class Task
    {
    protected:
        bool start(const char* name)
        {
            return !m_handle && xTaskCreatePinnedToCore(
                [](void* data) 
                {
                    auto instance = static_cast<Task*>(data);
                    instance->task();
                    instance->stop();
                },
                name, stack, this, priority, &m_handle, core
            ) == pdPASS;
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

        bool isAlive() const { return m_handle != nullptr; }
        TaskHandle_t getHandle() const  { return m_handle; }

        virtual void task() = 0;

    private:
        TaskHandle_t m_handle = nullptr;
    };
}