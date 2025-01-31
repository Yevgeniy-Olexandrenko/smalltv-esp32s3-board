#include "BackgroundTask.h"

BackgroundTask::BackgroundTask(const char* name, uint32_t stack)
    : _name(name)
    , _stack(stack)
    , _handle(nullptr)
    , _mutex(xSemaphoreCreateMutex())
{}

BackgroundTask::~BackgroundTask()
{
    end();
    vSemaphoreDelete(_mutex);
}

void BackgroundTask::begin()
{
    MutexLockGuard lock(_mutex);
    if (_handle == nullptr)
    {
        BaseType_t result = xTaskCreatePinnedToCore(
            BackgroundTask::task, // Task entry function (static)
            _name,                // Task name
            _stack,               // Stack size
            this,                 // Parameter to pass to the task
            1,                    // Task priority
            &_handle,             // Pointer to store task handle
            APP_CPU_NUM           // CPU core (on ESP32-S3 usually 1 for Arduino)
        );

        if (result != pdPASS)
        {
            _handle = nullptr;
            log_e("Failed to create task: %s", _name);
        }
    }
}

void BackgroundTask::suspend()
{
    MutexLockGuard lock(_mutex);
    if (_handle) vTaskSuspend(_handle);
}

void BackgroundTask::resume()
{
    MutexLockGuard lock(_mutex);
    if (_handle) vTaskResume(_handle);
}

void BackgroundTask::end()
{
    // notify task to complete peacefully
    TaskHandle_t handle = nullptr;
    {
        MutexLockGuard lock(_mutex);
        handle  = _handle; // save handle
        _handle = nullptr; // notify task
    }

    // wait for task completion
    for (int i = 100; i > 0; --i)
    {
        eTaskState state = eTaskGetState(handle);
        if (state == eInvalid || state == eDeleted) return;
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // kill task if it does not obey us
    log_e("Failed to end task gracefully: %s", _name);
    vTaskDelete(handle);
}

bool BackgroundTask::alive() const
{
    MutexLockGuard lock(_mutex);
    return (_handle != nullptr);
}

void BackgroundTask::task(void* arg)
{
    if (auto task = static_cast<BackgroundTask*>(arg))
    {
        if (task->init())
        {
            while(task->alive() && task->loop());
            task->free();
        }
        vTaskDelete(nullptr);
    }
}
