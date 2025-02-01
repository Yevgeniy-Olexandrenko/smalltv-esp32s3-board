#pragma once

namespace audio
{
    using MetadataCallback = void (*)(void* userData, const char* type, bool isUnicode, const char* string);

    class Metadata
    {
    public:
        Metadata() : _callback(nullptr), _userData(nullptr) {}

        void setCallback(MetadataCallback callback, void* userData) 
        { 
            _callback = callback;
            _userData = userData; 
        }

        void operator() (const char* type, bool isUnicode, const char* string)
        {
            if (_callback) _callback(_userData, type, isUnicode, string); 
        }

    private:
        MetadataCallback _callback;
        void* _userData;
    };
}
