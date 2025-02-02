#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include "Thread.h"

namespace task
{
    Thread::Thread(
        std::function<void()> func,
        uint32_t stackSize, UBaseType_t priority, BaseType_t coreId, const char *name)
        : _func(std::move(func)) 
        , _taskHandle(nullptr)
        , _joinable(true)
    {
        _doneSemaphore = xSemaphoreCreateBinary();
        if (!_doneSemaphore) ::abort();

        BaseType_t res = xTaskCreatePinnedToCore(
            TaskEntry, name, stackSize, this, priority, &_taskHandle, coreId);
        if (res != pdPASS)
        {
            vSemaphoreDelete(_doneSemaphore);
            ::abort();
        }
    }

    Thread::~Thread()
    {
        if (joinable()) ::abort();
        if (_doneSemaphore) 
            vSemaphoreDelete(_doneSemaphore);
    }

    Thread::Thread(Thread &&other) noexcept
        : _func(std::move(other._func))
        , _doneSemaphore(other._doneSemaphore)
        , _taskHandle(other._taskHandle)
        , _joinable(other._joinable)
    {
        other._doneSemaphore = nullptr;
        other._taskHandle = nullptr;
        other._joinable = false;
    }

    Thread &Thread::operator=(Thread &&other) noexcept
    {
        if (this != &other)
        {
            if (joinable() && _taskHandle) ::abort();
            
            _func = std::move(other._func);
            _doneSemaphore = other._doneSemaphore;
            _taskHandle = other._taskHandle;
            _joinable = other._joinable;
            
            other._doneSemaphore = nullptr;
            other._taskHandle = nullptr;
            other._joinable = false;
        }
        return *this;
    }

    void Thread::swap(Thread &other) noexcept
    {
        _func.swap(other._func);
        std::swap(_doneSemaphore, other._doneSemaphore);
        std::swap(_taskHandle, other._taskHandle);
        std::swap(_joinable, other._joinable);
    }

    void Thread::join()
    {
        if (joinable() && _taskHandle)
        {
            xSemaphoreTake(_doneSemaphore, portMAX_DELAY);
            detach();
        }
    }

    void Thread::detach()
    {
        _joinable = false;
        _taskHandle = nullptr;
    }

    bool Thread::joinable() const
    {
        return _joinable;
    }

    TaskHandle_t Thread::getNativeHandle() const
    {
        return _taskHandle;
    }

    const char *Thread::getName() const
    {
        return (_taskHandle ? pcTaskGetName(_taskHandle) : nullptr);
    }

    uintptr_t Thread::getId() const
    {
        return reinterpret_cast<uintptr_t>(_taskHandle);
    }

    void Thread::TaskEntry(void *param)
    {
        Thread *self = static_cast<Thread *>(param);
        if (self->_func) self->_func();
        if (self->_doneSemaphore)
            xSemaphoreGive(self->_doneSemaphore);
        vTaskDelete(NULL); // end the task
    }
}
