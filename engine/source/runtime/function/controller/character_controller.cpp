#include "runtime/function/controller/character_controller.h"

#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/physics/physics_system.h"
#include "runtime/function/global/global_context.h"

namespace Pilot
{
    Vector3 CharacterController::move(const Vector3& current_position, const Vector3& displacement)
    {
        Vector3 desired_position = current_position + displacement;
        if (g_runtime_global_context.m_physics_system->overlapByCapsule(desired_position, m_capsule))
        {
            desired_position = current_position;
        }

        return desired_position;
    }
} // namespace Pilot
