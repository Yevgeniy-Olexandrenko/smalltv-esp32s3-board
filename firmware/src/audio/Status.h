#pragma once

namespace audio
{
    using StatusCallback = void (*)(void* userData, int code, const char* string);

    class Status
    {
    public:
        Status() : _callback(nullptr), _userData(nullptr) {}

        void setCallback(StatusCallback callback, void* userData)
        {
            _callback = callback;
            _userData = userData;
        }

        void operator() (int code, const char* string)
        {
            if (_callback) _callback(_userData, code, string);
        }

    private:
        StatusCallback _callback;
        void* _userData;
    };
}