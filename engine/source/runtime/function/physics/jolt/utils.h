#pragma once

#include "core/base/macro.h"
#include "core/math/matrix4.h"
#include "core/math/quaternion.h"
#include "core/math/vector3.h"

#include "Jolt/Jolt.h"

#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h"

namespace JPH
{
    class Shape;
}

namespace Piccolo
{
    class RigidBodyShape;

    namespace Layers
    {
        static constexpr uint8_t UNUSED1 = 0; // 4 unused values so that broadphase layers values don't match with
                                              // object layer values (for testing purposes)
        static constexpr uint8_t UNUSED2    = 1;
        static constexpr uint8_t UNUSED3    = 2;
        static constexpr uint8_t UNUSED4    = 3;
        static constexpr uint8_t NON_MOVING = 4;
        static constexpr uint8_t MOVING     = 5;
        static constexpr uint8_t DEBRIS     = 6; // Example: Debris collides only with NON_MOVING
        static constexpr uint8_t SENSOR     = 7; // Sensors only collide with MOVING objects
        static constexpr uint8_t NUM_LAYERS = 8;
    }; // namespace Layers

    /// Broadphase layers
    namespace BroadPhaseLayers
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr JPH::BroadPhaseLayer DEBRIS(2);
        static constexpr JPH::BroadPhaseLayer SENSOR(3);
        static constexpr JPH::BroadPhaseLayer UNUSED(4);
        static constexpr uint32_t             NUM_LAYERS(5);
    }; // namespace BroadPhaseLayers

    /// BroadPhaseLayerInterface implementation
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl();

        uint32_t GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            ASSERT(inLayer < Layers::NUM_LAYERS);
            return m_object_to_broad_phase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

    private:
        JPH::BroadPhaseLayer m_object_to_broad_phase[Layers::NUM_LAYERS];
    };

    /// Function that determines if two object layers can collide
    bool ObjectCanCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2);

    /// Function that determines if two broadphase layers can collide
    bool BroadPhaseCanCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2);

    inline JPH::Vec3 toVec3(Vector3 v) { return {v.x, v.y, v.z}; }
    inline Vector3   toVec3(JPH::Vec3 v) { return {v.GetX(), v.GetY(), v.GetZ()}; }

    inline JPH::Vec4 toVec4(Vector4 v) { return {v.x, v.y, v.z, v.w}; }
    inline Vector4   toVec4(JPH::Vec4 v) { return {v.GetX(), v.GetY(), v.GetZ(), v.GetW()}; }

    inline JPH::Quat  toQuat(Quaternion q) { return {q.x, q.y, q.z, q.w}; }
    inline Quaternion toQuat(JPH::Quat q) { return {q.GetW(), q.GetX(), q.GetY(), q.GetZ()}; }

    JPH::Mat44 toMat44(const Matrix4x4& m);

    Matrix4x4 toMat44(const JPH::Mat44& m);

    JPH::Shape* toShape(const RigidBodyShape& shape, const Vector3& scale);

} // namespace Piccolo