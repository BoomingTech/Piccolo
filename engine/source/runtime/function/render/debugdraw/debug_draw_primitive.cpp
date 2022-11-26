#include "debug_draw_primitive.h"
namespace Piccolo
{
    bool Piccolo::DebugDrawPrimitive::isTimeOut(float delta_time)
    {
        if (m_time_type == _debugDrawTimeType_infinity)
        {
            return false;
        }
        else if (m_time_type == _debugDrawTimeType_one_frame)
        {
            if (!m_rendered)
            {
                m_rendered = true;
                return false;
            }
            else return true;
        }
        else
        {
            m_life_time -= delta_time;
            return (m_life_time < 0.0f);
        }
        return false;
    }

    void Piccolo::DebugDrawPrimitive::setTime(float in_life_time)
    {
        if (fabs(in_life_time - k_debug_draw_infinity_life_time) < 1e-6)
        {
            m_time_type = _debugDrawTimeType_infinity;
            m_life_time = 0.0f;
        }
        else if (fabs(in_life_time - k_debug_draw_one_frame) < 1e-6)
        {
            m_time_type = _debugDrawTimeType_one_frame;
            m_life_time = 0.03f;
        }
        else
        {
            m_time_type = _debugDrawTimeType_common;
            m_life_time = in_life_time;
        }
    }
}