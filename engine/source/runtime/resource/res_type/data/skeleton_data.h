#pragma once
#include "runtime/core/math/transform.h"
#include "runtime/core/meta/reflection/reflection.h"

#include <string>
#include <vector>
namespace Piccolo
{

    REFLECTION_TYPE(RawBone)
    CLASS(RawBone, Fields)
    {
        REFLECTION_BODY(RawBone);

    public:
        std::string m_name;
        int         m_index;
        Transform   m_binding_pose;
        Matrix4x4_  m_tpose_matrix;
        int         m_parent_index;
    };

    REFLECTION_TYPE(SkeletonData)
    CLASS(SkeletonData, Fields)
    {
        REFLECTION_BODY(SkeletonData);

    public:
        std::vector<RawBone> m_bones_map;
        bool                 m_is_flat = false; //"bone.index" equals index in bones_map
        int                  m_root_index;
        bool                 m_in_topological_order = false; // TODO: if not in topological order, we need to topology sort in skeleton
                                                             // build process
    };

} // namespace Piccolo