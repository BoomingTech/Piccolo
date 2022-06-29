#include "runtime/function/physics/jolt/utils.h"

#include "runtime/resource/res_type/components/rigid_body.h"

#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/StaticCompoundShape.h"

namespace Piccolo
{
    BPLayerInterfaceImpl::BPLayerInterfaceImpl()
    {
        // Create a mapping table from object to broad phase layer
        m_object_to_broad_phase[Layers::UNUSED1]    = BroadPhaseLayers::UNUSED;
        m_object_to_broad_phase[Layers::UNUSED2]    = BroadPhaseLayers::UNUSED;
        m_object_to_broad_phase[Layers::UNUSED3]    = BroadPhaseLayers::UNUSED;
        m_object_to_broad_phase[Layers::UNUSED4]    = BroadPhaseLayers::UNUSED;
        m_object_to_broad_phase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        m_object_to_broad_phase[Layers::MOVING]     = BroadPhaseLayers::MOVING;
        m_object_to_broad_phase[Layers::DEBRIS]     = BroadPhaseLayers::DEBRIS;
        m_object_to_broad_phase[Layers::SENSOR]     = BroadPhaseLayers::SENSOR;
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const
    {
        switch ((JPH::BroadPhaseLayer::Type)inLayer)
        {
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
                return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
                return "MOVING";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::DEBRIS:
                return "DEBRIS";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::SENSOR:
                return "SENSOR";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::UNUSED:
                return "UNUSED";
            default:
                ASSERT(false);
                return "INVALID";
        }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

    bool ObjectCanCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2)
    {
        switch (inObject1)
        {
            case Layers::UNUSED1:
            case Layers::UNUSED2:
            case Layers::UNUSED3:
            case Layers::UNUSED4:
                return false;
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING || inObject2 == Layers::DEBRIS;
            case Layers::MOVING:
                return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING || inObject2 == Layers::SENSOR;
            case Layers::DEBRIS:
                return inObject2 == Layers::NON_MOVING;
            case Layers::SENSOR:
                return inObject2 == Layers::MOVING;
            default:
                ASSERT(false);
                return false;
        }
    }

    bool BroadPhaseCanCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2)
    {
        switch (inLayer1)
        {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING ||
                       inLayer2 == BroadPhaseLayers::SENSOR;
            case Layers::DEBRIS:
                return inLayer2 == BroadPhaseLayers::NON_MOVING;
            case Layers::SENSOR:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::UNUSED1:
            case Layers::UNUSED2:
            case Layers::UNUSED3:
                return false;
            default:
                ASSERT(false);
                return false;
        }
    }

    JPH::Mat44 toMat44(const Matrix4x4& m)
    {
        JPH::Vec4 cols[4];
        for (int i = 0; i < 4; i++)
        {
            cols[i] = JPH::Vec4(m.m_mat[0][i], m.m_mat[1][i], m.m_mat[2][i], m.m_mat[3][i]);
        }

        return {cols[0], cols[1], cols[2], cols[3]};
    }

    Matrix4x4 toMat44(const JPH::Mat44& m)
    {
        Vector4 cols[4];
        for (int i = 0; i < 4; i++)
        {
            cols[i] = toVec4(m.GetColumn4(i));
        }

        return Matrix4x4(cols[0], cols[1], cols[2], cols[3]).transpose();
    }

    JPH::Shape* toShape(const RigidBodyShape& shape, const Vector3& scale)
    {
        JPH::Shape* jph_shape = nullptr;

        const std::string shape_type_str = shape.m_geometry.getTypeName();
        if (shape_type_str == "Box")
        {
            const Box* box_geometry = static_cast<const Box*>(shape.m_geometry.getPtr());
            if (box_geometry)
            {
                JPH::Vec3 jph_box(scale.x * box_geometry->m_half_extents.x,
                                  scale.y * box_geometry->m_half_extents.y,
                                  scale.z * box_geometry->m_half_extents.z);
                jph_shape = new JPH::BoxShape(jph_box, 0.f);
            }
        }
        else if (shape_type_str == "Sphere")
        {
            const Sphere* sphere_geometry = static_cast<const Sphere*>(shape.m_geometry.getPtr());
            if (sphere_geometry)
            {
                jph_shape = new JPH::SphereShape((scale.x + scale.y + scale.z) / 3 *
                                                 sphere_geometry->m_radius);
            }
        }
        else if (shape_type_str == "Capsule")
        {
            const Capsule* capsule_geometry = static_cast<const Capsule*>(shape.m_geometry.getPtr());
            if (capsule_geometry)
            {
                jph_shape = new JPH::CapsuleShape(scale.z * capsule_geometry->m_half_height,
                                                  (scale.x + scale.y) / 2 * capsule_geometry->m_radius);
            }
        }
        else
        {
            LOG_ERROR("Unsupported Shape")
        }

        return jph_shape;
    }

} // namespace Piccolo
