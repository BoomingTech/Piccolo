#pragma once

#include "runtime/resource/res_type/data/animation_clip.h"
#include "runtime/resource/res_type/data/animation_skeleton_node_map.h"
#include "runtime/resource/res_type/data/blend_state.h"
#include "runtime/resource/res_type/data/skeleton_data.h"
#include "runtime/resource/res_type/data/skeleton_mask.h"

#include <map>
#include <memory>
#include <string>

namespace Piccolo
{
    class AnimationManager
    {
    private:
        static std::map<std::string, std::shared_ptr<SkeletonData>>  m_skeleton_definition_cache;
        static std::map<std::string, std::shared_ptr<AnimationClip>> m_animation_data_cache;
        static std::map<std::string, std::shared_ptr<AnimSkelMap>>   m_animation_skeleton_map_cache;
        static std::map<std::string, std::shared_ptr<BoneBlendMask>> m_skeleton_mask_cache;

    public:
        static std::shared_ptr<SkeletonData>  tryLoadSkeleton(std::string file_path);
        static std::shared_ptr<AnimationClip> tryLoadAnimation(std::string file_path);
        static std::shared_ptr<AnimSkelMap>   tryLoadAnimationSkeletonMap(std::string file_path);
        static std::shared_ptr<BoneBlendMask> tryLoadSkeletonMask(std::string file_path);
        static BlendStateWithClipData         getBlendStateWithClipData(const BlendState& blend_state);

        AnimationManager() = default;
    };

} // namespace Piccolo