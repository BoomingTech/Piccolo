#pragma once

#include "debug_draw_group.h"

namespace Piccolo
{
    class DebugDrawContext
    {
    public:
        std::vector<DebugDrawGroup*> m_debug_draw_groups;
        DebugDrawGroup* tryGetOrCreateDebugDrawGroup(const std::string& name);
        void clear();
        void tick(float delta_time);
    
    private:
        std::mutex m_mutex;
        void removeDeadPrimitives(float delta_time);
    };

}