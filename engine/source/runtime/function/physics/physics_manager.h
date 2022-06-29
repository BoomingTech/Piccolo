#pragma once

#include "runtime/core/math/vector3.h"

#include <memory>
#include <vector>

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
class Renderer;
class Font;

namespace JPH
{
    class DebugRenderer;
}
#endif

namespace Piccolo
{
    class PhysicsScene;

    class PhysicsManager
    {
    public:
        void initialize();
        void clear();

        std::weak_ptr<PhysicsScene> createPhysicsScene(const Vector3& gravity);
        void                        deletePhysicsScene(std::weak_ptr<PhysicsScene> physics_scene);

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        void renderPhysicsWorld(float delta_time);
#endif

    protected:
        std::vector<std::shared_ptr<PhysicsScene>> m_scenes;

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        Renderer* m_renderer {nullptr};
        Font*     m_font {nullptr};

        JPH::DebugRenderer* m_debug_renderer {nullptr};
#endif
    };
} // namespace Piccolo
