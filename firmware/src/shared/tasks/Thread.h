#pragma once

#include <functional>
#include <freertos/task.h>
#include <freertos/semphr.h>

namespace task
{
    class Thread
    {
    public:
        // Copying is disabled.
        Thread(const Thread &) = delete;
        Thread &operator=(const Thread &) = delete;

        // Constructor:
        // - func: callable (without parameters); wrap with lambda if parameters are needed.
        // - stackSize: task stack size (in bytes), default 4096.
        // - priority: task priority, default tskIDLE_PRIORITY + 1.
        // - coreId: core affinity of type BaseType_t, default tskNO_AFFINITY.
        // - name: string name of the task, default "ThreadTask".
        Thread(std::function<void()> func,
            uint32_t stackSize = 4096,
            UBaseType_t priority = tskIDLE_PRIORITY + 1,
            BaseType_t coreId = tskNO_AFFINITY,
            const char *name = "ThreadTask");

        // Destructor: aborts if the thread is still joinable.
        ~Thread();

        Thread(Thread &&other) noexcept;
        Thread &operator=(Thread &&other) noexcept;
        void swap(Thread &other) noexcept;

        void join();
        void detach();
        bool joinable() const;

        TaskHandle_t getNativeHandle() const;
        const char* getName() const;
        uintptr_t getId() const;

    private:
        std::function<void()> _func;
        SemaphoreHandle_t _doneSemaphore;
        TaskHandle_t _taskHandle;
        bool _joinable;

        // Entry point for the FreeRTOS task.
        static void TaskEntry(void *param);
    };
}
