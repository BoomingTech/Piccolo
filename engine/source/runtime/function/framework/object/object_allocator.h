#pragma once

#include "runtime/core/base/macro.h"

#include <limits>

namespace Pilot
{
    using GObjectID = size_t;

    constexpr GObjectID k_invalid_go_id = std::numeric_limits<size_t>::max();

    class ObjectIDAllocator
    {
    public:
        static GObjectID alloc()
        {
            std::atomic<GObjectID> ret = m_next_id.load();
            m_next_id++;
            if (m_next_id >= k_invalid_go_id)
            {
                LOG_FATAL("gobject id overflow");
            }

            return ret;
        }

    private:
        static std::atomic<GObjectID> m_next_id;
    };
} // namespace Pilot
