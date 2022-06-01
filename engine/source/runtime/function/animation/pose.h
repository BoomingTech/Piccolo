#pragma once
#include "runtime/resource/res_type/data/animation_clip.h"
#include "runtime/resource/res_type/data/animation_skeleton_node_map.h"
#include "runtime/resource/res_type/data/blend_state.h"
namespace Pilot
{
    class AnimationPose
    {
        static void extractFromClip (std::vector<Transform>&bones, const AnimationClip& clip, float ratio);
    public:
        std::vector<Transform> m_bone_poses;
        bool                   m_reorder {false};
        std::vector<int>       m_bone_indexs;
        BoneBlendWeight        m_weight;
        AnimationPose();
        AnimationPose(const AnimationClip& clip, float ratio, const AnimSkelMap& animSkelMap);
        AnimationPose(const AnimationClip& clip, const BoneBlendWeight& weight, float ratio);
        AnimationPose(const AnimationClip& clip, const BoneBlendWeight& weight, float ratio, const AnimSkelMap& animSkelMap);
        void blend(const AnimationPose& pose);
    };
} // namespace Pilot
