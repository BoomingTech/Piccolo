#pragma once

#include "runtime/resource/res_type/data/animation_clip.h"
#include "runtime/resource/res_type/data/animation_skeleton_node_map.h"
#include "runtime/resource/res_type/data/skeleton_data.h"
#include "runtime/resource/res_type/data/skeleton_mask.h"

#include <memory>
#include <string>

namespace Piccolo
{
    class AnimationLoader
    {
    public:
        std::shared_ptr<AnimationClip> loadAnimationClipData(std::string animation_clip_url);
        std::shared_ptr<SkeletonData>  loadSkeletonData(std::string skeleton_data_url);
        std::shared_ptr<AnimSkelMap>   loadAnimSkelMap(std::string anim_skel_map_url);
        std::shared_ptr<BoneBlendMask> loadSkeletonMask(std::string skeleton_mask_file_url);
    };
} // namespace Piccolo