#pragma once
#include "runtime/core/math/transform.h"
#include "runtime/core/meta/reflection/reflection.h"

#include <string>
#include <vector>

namespace Piccolo
{

    REFLECTION_TYPE(AnimNodeMap)
    CLASS(AnimNodeMap, Fields)
    {
        REFLECTION_BODY(AnimNodeMap);

    public:
        std::vector<std::string> m_convert;
    };

    REFLECTION_TYPE(AnimationChannel)
    CLASS(AnimationChannel, Fields)
    {
        REFLECTION_BODY(AnimationChannel);

    public:
        std::string             m_name;
        std::vector<Vector3>    m_position_keys;
        std::vector<Quaternion> m_rotation_keys;
        std::vector<Vector3>    m_scaling_keys;
    };

    REFLECTION_TYPE(AnimationClip)
    CLASS(AnimationClip, Fields)
    {
        REFLECTION_BODY(AnimationClip);

    public:
        int                           m_total_frame {0};
        int                           m_node_count {0};
        std::vector<AnimationChannel> m_node_channels;
    };

    REFLECTION_TYPE(AnimationAsset)
    CLASS(AnimationAsset, Fields)
    {
        REFLECTION_BODY(AnimationAsset);

    public:
        AnimNodeMap   m_node_map;
        AnimationClip m_clip_data;
        std::string   m_skeleton_file_path;
    };

} // namespace Piccolo