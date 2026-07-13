#include "runtime/function/framework/object/object_id_allocator.h"

#include "core/base/macro.h"

namespace Piccolo
{
    std::atomic<GObjectID> ObjectIDAllocator::m_next_id {0};

    GObjectID ObjectIDAllocator::alloc()
    {
        GObjectID new_object_ret = m_next_id++;
        if (new_object_ret >= k_invalid_gobject_id - 1)
        {
            LOG_FATAL("gobject id overflow");
        }

        return new_object_ret;
    }

} // namespace Piccolo





