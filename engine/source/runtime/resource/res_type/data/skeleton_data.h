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
        std::string name;
        int         index;
        Transform   binding_pose;
        Matrix4x4_  tpose_matrix;
        int         parent_index;
    };

    REFLECTION_TYPE(SkeletonData)
    CLASS(SkeletonData, Fields)
    {
        REFLECTION_BODY(SkeletonData);

    public:
        std::vector<RawBone> bones_map;
        bool                 is_flat = false; //"bone.index" equals index in bones_map
        int                  root_index;
        bool in_topological_order = false; // TODO: if not in topological order, we need to topology sort in skeleton
                                           // build process
    };

} // namespace Piccolo