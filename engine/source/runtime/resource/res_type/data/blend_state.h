#pragma once
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/resource/res_type/data/animation_clip.h"
#include "runtime/resource/res_type/data/animation_skeleton_node_map.h"
#include <string>
#include <vector>
namespace Piccolo
{

    REFLECTION_TYPE(BoneBlendWeight)
    CLASS(BoneBlendWeight, Fields)
    {
        REFLECTION_BODY(BoneBlendWeight);

    public:
        std::vector<float> blend_weight;
    };

    REFLECTION_TYPE(BlendStateWithClipData)
    CLASS(BlendStateWithClipData, Fields)
    {
        REFLECTION_BODY(BlendStateWithClipData);

    public:
        int                          clip_count;
        std::vector<AnimationClip>   blend_clip;
        std::vector<AnimSkelMap>     blend_anim_skel_map;
        std::vector<BoneBlendWeight> blend_weight;
        std::vector<float>           blend_ratio;
    };

    REFLECTION_TYPE(BlendState)
    CLASS(BlendState, Fields)
    {
        REFLECTION_BODY(BlendState);

    public:
        int                      clip_count;
        std::vector<std::string> blend_clip_file_path;
        std::vector<float>       blend_clip_file_length;
        std::vector<std::string> blend_anim_skel_map_path;
        std::vector<float>       blend_weight;
        std::vector<std::string> blend_mask_file_path;
        std::vector<float>       blend_ratio;
    };

} // namespace Piccolo