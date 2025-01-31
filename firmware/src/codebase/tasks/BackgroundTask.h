#pragma once

#include <Arduino.h>
#include "MutexLockGuard.h"

class BackgroundTask
{
public:
    BackgroundTask(const char* name, uint32_t stack = 2048);
    virtual ~BackgroundTask();

    virtual void begin();
    virtual void suspend();
    virtual void resume();
    virtual void end();
    bool alive() const;

protected:
    static  void task(void* arg);
    virtual bool init() { return true; };
    virtual bool loop() = 0;
    virtual void free() {};

protected:
    const char* _name;
    uint32_t _stack;
    TaskHandle_t _handle;
    SemaphoreHandle_t _mutex;
};
