#include "runtime/function/animation/pose.h"

namespace Piccolo
{
    AnimationPose::AnimationPose() { m_reorder = false; }

    AnimationPose::AnimationPose(const AnimationClip& clip, float ratio, const AnimSkelMap& animSkelMap)
    {
        m_bone_indexs = animSkelMap.m_convert;
        m_reorder     = true;
        extractFromClip(m_bone_poses, clip, ratio);
        m_weight.m_blend_weight.resize(m_bone_poses.size());
        for (auto& weight : m_weight.m_blend_weight)
        {
            weight = 1.f;
        }
    }

    AnimationPose::AnimationPose(const AnimationClip& clip, const BoneBlendWeight& weight, float ratio)
    {
        m_weight  = weight;
        m_reorder = false;
        extractFromClip(m_bone_poses, clip, ratio);
    }

    AnimationPose::AnimationPose(const AnimationClip& clip, const BoneBlendWeight& weight, float ratio, const AnimSkelMap& animSkelMap)
    {
        m_weight      = weight;
        m_bone_indexs = animSkelMap.m_convert;
        m_reorder     = true;
        extractFromClip(m_bone_poses, clip, ratio);
    }

    void AnimationPose::extractFromClip(std::vector<Transform>& bones, const AnimationClip& clip, float ratio)
    {
        bones.resize(clip.m_node_count);

        float exact_frame        = ratio * (clip.m_total_frame - 1);
        int   current_frame_low  = floor(exact_frame);
        int   current_frame_high = ceil(exact_frame);
        float lerp_ratio         = exact_frame - current_frame_low;
        for (int i = 0; i < clip.m_node_count; i++)
        {
            const AnimationChannel& channel = clip.m_node_channels[i];
            bones[i].m_position = Vector3::lerp(channel.m_position_keys[current_frame_low], channel.m_position_keys[current_frame_high], lerp_ratio);
            bones[i].m_scale    = Vector3::lerp(channel.m_scaling_keys[current_frame_low], channel.m_scaling_keys[current_frame_high], lerp_ratio);
            bones[i].m_rotation = Quaternion::nLerp(lerp_ratio, channel.m_rotation_keys[current_frame_low], channel.m_rotation_keys[current_frame_high], true);
        }
    }

    void AnimationPose::blend(const AnimationPose& pose)
    {
        for (int i = 0; i < m_bone_poses.size(); i++)
        {
            auto&       bone_trans_one = m_bone_poses[i];
            const auto& bone_trans_two = pose.m_bone_poses[i];

            float sum_weight   = 0;
            float other_weight = pose.m_weight.m_blend_weight[i];
            float this_weight  = m_weight.m_blend_weight[i];
            sum_weight         = other_weight + this_weight;

            if (sum_weight != 0)
            {
                float cur_weight           = this_weight / sum_weight;
                m_weight.m_blend_weight[i] = sum_weight;
                bone_trans_one.m_position  = bone_trans_one.m_position * cur_weight + bone_trans_two.m_position * (1 - cur_weight);
                bone_trans_one.m_scale     = bone_trans_one.m_scale * cur_weight + bone_trans_two.m_scale * (1 - cur_weight);
                bone_trans_one.m_rotation  = Quaternion::sLerp(1 - cur_weight, bone_trans_one.m_rotation, bone_trans_two.m_rotation, true);
            }
        }
    }
} // namespace Piccolo
