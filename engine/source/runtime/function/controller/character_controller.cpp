#include "runtime/function/controller/character_controller.h"

#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/physics/physics_system.h"

namespace Pilot
{
    Controller::~Controller() = default;

    CharacterController::~CharacterController() {}

    Vector3 CharacterController::move(const Vector3& current_position, const Vector3& displacement)
    {
        Vector3 desired_position = current_position + displacement;
        if (PhysicsSystem::getInstance().overlapByCapsule(desired_position, m_capsule))
        {
            desired_position = current_position;
        }

        return desired_position;
    }
} // namespace Pilot
