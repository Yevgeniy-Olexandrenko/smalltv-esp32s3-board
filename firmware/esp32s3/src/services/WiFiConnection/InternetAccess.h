#pragma once

#include "core/Core.h"

namespace service::details
{
    class InternetAccess
    {
        constexpr static auto CHECK_PERIOD_SEC  = 20;
        constexpr static auto CHECK_TIMEOUT_SEC = 2;

    public:
        bool available() const;
        void update();

    private:
        bool m_available = false;
        core::Timer m_timer;
    };
}
