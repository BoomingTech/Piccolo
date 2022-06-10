#include "runtime/function/controller/character_controller.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/character/character.h"
#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/framework/level/level.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/physics/physics_scene.h"

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
#include "TestFramework.h"

#include "TestFramework/Renderer/DebugRendererImp.h"
#include "TestFramework/Renderer/Font.h"
#include "TestFramework/Renderer/Renderer.h"
#include "TestFramework/Utils/Log.h"
#endif

namespace Pilot
{
    CharacterController::CharacterController(const Capsule& capsule) : m_capsule(capsule)
    {
        m_rigidbody_shape                                    = RigidBodyShape();
        m_rigidbody_shape.m_geometry                         = PILOT_REFLECTION_NEW(Capsule);
        *static_cast<Capsule*>(m_rigidbody_shape.m_geometry) = m_capsule;

        m_rigidbody_shape.m_type = RigidBodyShapeType::capsule;

        Quaternion orientation;
        orientation.fromAngleAxis(Radian(Degree(90.f)), Vector3::UNIT_X);

        m_rigidbody_shape.m_local_transform =
            Transform(Vector3(0, 0, capsule.m_half_height + capsule.m_radius), orientation, Vector3::UNIT_SCALE);
    }

    Vector3 CharacterController::move(const Vector3& current_position, const Vector3& displacement)
    {
        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        std::vector<PhysicsHitInfo> hits;

        Transform world_transform =
            Transform(current_position + 0.1f * Vector3::UNIT_Z, Quaternion::IDENTITY, Vector3::UNIT_SCALE);

        Vector3 vertical_displacement   = displacement.z * Vector3::UNIT_Z;
        Vector3 horizontal_displacement = Vector3(displacement.x, displacement.y, 0.f);

        Vector3 vertical_direction   = vertical_displacement.normalisedCopy();
        Vector3 horizontal_direction = horizontal_displacement.normalisedCopy();

        Vector3 final_position = current_position;

        m_is_touch_ground = physics_scene->sweep(
            m_rigidbody_shape, world_transform.getMatrix(), Vector3::NEGATIVE_UNIT_Z, 0.105f, hits);

        hits.clear();

        std::shared_ptr<Level> current_level = g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();

        std::shared_ptr<Character> current_character = current_level->getCurrentActiveCharacter().lock();
        if (current_character == nullptr)
            return final_position;

        Vector3 characterUpDirection = current_character->getRotation() * Vector3::UNIT_Z;
        characterUpDirection         = characterUpDirection.normalisedCopy();
        Vector3 startPosition        = current_character->getPosition();

        Vector3 characterForwardDirection = current_character->getRotation() * Vector3::UNIT_Y;
        characterForwardDirection         = characterForwardDirection.normalisedCopy();

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        Vector3 targetPosition = startPosition + characterForwardDirection * 10.0f;
        DebugRenderer::sInstance->DrawLine(JPH::Float3(startPosition.x, startPosition.y, startPosition.z),
                                           JPH::Float3(targetPosition.x, targetPosition.y, targetPosition.z),
                                           JPH::Color::sYellow);
#endif

        world_transform.m_position -= 0.1f * Vector3::UNIT_Z;

        // vertical pass
        if (physics_scene->sweep(m_rigidbody_shape,
                                 world_transform.getMatrix(),
                                 vertical_direction,
                                 vertical_displacement.length(),
                                 hits))
        {
            final_position += hits[0].hit_distance * vertical_direction;
        }
        else
        {
            final_position += vertical_displacement;
        }

        hits.clear();

        world_transform.m_position += 0.1f * Vector3::UNIT_Z;

        // side pass
        if (physics_scene->sweep(m_rigidbody_shape,
                                 world_transform.getMatrix(),
                                 horizontal_direction,
                                 horizontal_displacement.length(),
                                 hits))
        {

            Vector3 newDirection = Vector3::ZERO;
            float   high;
            world_transform.m_position += 0.2f * Vector3::UNIT_Z;
            if (physics_scene->isOverlap(m_rigidbody_shape, world_transform.getMatrix(), high))
            {
                if (high < MIN_STEP)
                {
                    newDirection = (characterUpDirection + characterForwardDirection) * 0.02f;
                }
                else
                {
                    newDirection = hits[0].hit_normal.normalisedCopy().crossProduct(characterUpDirection);
                    newDirection = newDirection.normalisedCopy() * 0.005f;
                }
            }

            final_position += newDirection;
        }
        else
        {
            final_position += horizontal_displacement;
        }

        return final_position;
    }

} // namespace Pilot
