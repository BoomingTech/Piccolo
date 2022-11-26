#include "debug_draw_context.h"

namespace Piccolo
{
    DebugDrawGroup* DebugDrawContext::tryGetOrCreateDebugDrawGroup(const std::string& name)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        
        size_t debug_draw_group_count = m_debug_draw_groups.size();
        for (size_t debug_draw_group_index = 0; debug_draw_group_index < debug_draw_group_count; debug_draw_group_index++)
        {
            DebugDrawGroup* debug_draw_group = m_debug_draw_groups[debug_draw_group_index];
            if (debug_draw_group->getName() == name)
            {
                return debug_draw_group;
            }
        }

        DebugDrawGroup* new_debug_draw_group = new DebugDrawGroup;
        new_debug_draw_group->initialize();
        new_debug_draw_group->setName(name);
        m_debug_draw_groups.push_back(new_debug_draw_group);

        return new_debug_draw_group;
    }

    void DebugDrawContext::clear()
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        size_t debug_draw_group_count = m_debug_draw_groups.size();
        for (size_t debug_draw_group_index = 0; debug_draw_group_index < debug_draw_group_count; debug_draw_group_index++)
        {
            delete m_debug_draw_groups[debug_draw_group_index];
        }

        m_debug_draw_groups.clear();
    }

    void DebugDrawContext::tick(float delta_time)
    {
        removeDeadPrimitives(delta_time);
    }

    void DebugDrawContext::removeDeadPrimitives(float delta_time)
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        size_t debug_draw_group_count = m_debug_draw_groups.size();
        for (size_t debug_draw_group_index = 0; debug_draw_group_index < debug_draw_group_count; debug_draw_group_index++)
        {
            if (m_debug_draw_groups[debug_draw_group_index] == nullptr)continue;
            m_debug_draw_groups[debug_draw_group_index]->removeDeadPrimitives(delta_time);
        }
    }
}