#include "runtime/function/framework/object/object_id_allocator.h"

#include "core/base/macro.h"

namespace Piccolo
{
    std::atomic<GObjectID> ObjectIDAllocator::m_next_id {0};

    GObjectID ObjectIDAllocator::alloc()
    {
        // 使用松散内存顺序（std::memory_order_relaxed）执行原子自增并返回旧值，性能更优
        GObjectID new_object_ret = m_next_id.fetch_add(1, std::memory_order_relaxed);
        
        // 由于 new_object_ret 是自增前的值，判断自增后的值（new_object_ret + 1）是否溢出，
        // 等价于判断 new_object_ret 是否大于等于 k_invalid_gobject_id - 1
        if (new_object_ret >= k_invalid_gobject_id - 1)
        {
            LOG_FATAL("gobject id overflow");
        }

        return new_object_ret;
    }

} // namespace Piccolo






