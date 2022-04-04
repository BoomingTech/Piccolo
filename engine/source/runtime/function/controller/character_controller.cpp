#include "runtime/function/controller/character_controller.h"

#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/physics/physics_system.h"

namespace Pilot
{
    CharacterController::~CharacterController() {}

    Vector3 CharacterController::move(const Vector3& current_position, const Vector3& displacement)
    {
        Vector3 desired_position = current_position + displacement;
        if (PhysicsSystem::getInstance().overlap(current_position + displacement, m_half_extent))
        {
            desired_position = current_position;
        }

        return desired_position;
    }
} // namespace Pilot
