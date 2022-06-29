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
        std::vector<std::string> convert;
    };

    REFLECTION_TYPE(AnimationChannel)
    CLASS(AnimationChannel, Fields)
    {
        REFLECTION_BODY(AnimationChannel);

    public:
        std::string             name;
        std::vector<Vector3>    position_keys;
        std::vector<Quaternion> rotation_keys;
        std::vector<Vector3>    scaling_keys;
    };

    REFLECTION_TYPE(AnimationClip)
    CLASS(AnimationClip, Fields)
    {
        REFLECTION_BODY(AnimationClip);

    public:
        int                           total_frame {0};
        int                           node_count {0};
        std::vector<AnimationChannel> node_channels;
    };

    REFLECTION_TYPE(AnimationAsset)
    CLASS(AnimationAsset, Fields)
    {
        REFLECTION_BODY(AnimationAsset);

    public:
        AnimNodeMap   node_map;
        AnimationClip clip_data;
        std::string   skeleton_file_path;
    };

} // namespace Piccolo