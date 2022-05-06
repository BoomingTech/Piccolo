#include "runtime/resource/res_type/components/motor.h"

#include "runtime/core/base/macro.h"

namespace Pilot
{
    MotorComponentRes::MotorComponentRes(const MotorComponentRes& res) :
        m_move_speed(res.m_move_speed), m_jump_height(res.m_jump_height),
        m_max_move_speed_ratio(res.m_max_move_speed_ratio), m_max_sprint_speed_ratio(res.m_max_sprint_speed_ratio),
        m_move_acceleration(res.m_move_acceleration), m_sprint_acceleration(res.m_sprint_acceleration)
    {
        if (res.m_controller_config.getTypeName() == "PhysicsControllerConfig")
        {
            m_controller_config = PILOT_REFLECTION_NEW(PhysicsControllerConfig);
            PILOT_REFLECTION_DEEP_COPY(PhysicsControllerConfig, m_controller_config, res.m_controller_config);

            m_controller_type = ControllerType::physics;
        }
        else if (res.m_controller_config != nullptr)
        {
            m_controller_type = ControllerType::invalid;
            LOG_ERROR("invalid controller type, not able to move");
        }
    }

    MotorComponentRes::~MotorComponentRes() { PILOT_REFLECTION_DELETE(m_controller_config); }
} // namespace Pilot