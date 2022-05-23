#pragma once

#include "runtime/core/math/matrix4.h"
#include "runtime/function/framework/object/object_id_allocator.h"

#include <string>
#include <vector>

namespace Pilot
{
    REFLECTION_TYPE(GameObjectMeshDesc)
    STRUCT(GameObjectMeshDesc, Fields)
    {
        REFLECTION_BODY(GameObjectMeshDesc)
        std::string mesh_file;
    };

    REFLECTION_TYPE(SkeletonBindingDesc)
    STRUCT(SkeletonBindingDesc, Fields)
    {
        REFLECTION_BODY(SkeletonBindingDesc)
        std::string skeleton_binding_file;
    };

    REFLECTION_TYPE(SkeletonAnimationResultTransform)
    STRUCT(SkeletonAnimationResultTransform, WhiteListFields)
    {
        REFLECTION_BODY(SkeletonAnimationResultTransform)
        Matrix4x4 matrix;
    };

    REFLECTION_TYPE(SkeletonAnimationResult)
    STRUCT(SkeletonAnimationResult, Fields)
    {
        REFLECTION_BODY(SkeletonAnimationResult)
        std::vector<SkeletonAnimationResultTransform> transforms;
    };

    REFLECTION_TYPE(GameObjectMaterialDesc)
    STRUCT(GameObjectMaterialDesc, Fields)
    {
        REFLECTION_BODY(GameObjectMaterialDesc)
        std::string baseColorTextureFile;
        std::string metallicRoughnessTextureFile;
        std::string normalTextureFile;
        std::string occlusionTextureFile;
        std::string emissiveTextureFile;
        bool        with_texture {false};
    };

    REFLECTION_TYPE(GameObjectTransformDesc)
    STRUCT(GameObjectTransformDesc, WhiteListFields)
    {
        REFLECTION_BODY(GameObjectTransformDesc)
        Matrix4x4 transform_matrix {Matrix4x4::IDENTITY};
    };

    REFLECTION_TYPE(GameObjectPartDesc)
    STRUCT(GameObjectPartDesc, Fields)
    {
        REFLECTION_BODY(GameObjectPartDesc)
        GameObjectMeshDesc      mesh_desc;
        GameObjectMaterialDesc  material_desc;
        GameObjectTransformDesc transform_desc;
        bool                    with_animation {false};
        SkeletonBindingDesc     skeleton_binding_desc;
        SkeletonAnimationResult skeleton_animation_result;
    };

    constexpr size_t k_invalid_part_id = std::numeric_limits<size_t>::max();

    struct GameObjectPartId
    {
        GObjectID go_id {k_invalid_gobject_id};
        size_t    part_id {k_invalid_part_id};

        bool   operator==(const GameObjectPartId& rhs) const { return go_id == rhs.go_id && part_id == rhs.part_id; }
        size_t getHashValue() const { return go_id ^ (part_id << 1); }
        bool   isValid() const { return go_id != k_invalid_gobject_id && part_id != k_invalid_part_id; }
    };

    class GameObjectDesc
    {
    public:
        GameObjectDesc() : m_go_id(0) {}
        GameObjectDesc(size_t go_id, const std::vector<GameObjectPartDesc>& parts) :
            m_go_id(go_id), m_object_parts(parts)
        {}

        GObjectID                              getId() const { return m_go_id; }
        const std::vector<GameObjectPartDesc>& getObjectParts() const { return m_object_parts; }

    private:
        GObjectID                       m_go_id {k_invalid_gobject_id};
        std::vector<GameObjectPartDesc> m_object_parts;
    };
} // namespace Pilot

template<>
struct std::hash<Pilot::GameObjectPartId>
{
    size_t operator()(const Pilot::GameObjectPartId& rhs) const noexcept { return rhs.getHashValue(); }
};
