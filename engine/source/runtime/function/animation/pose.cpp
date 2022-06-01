#include "runtime/function/animation/pose.h"

using namespace Pilot;

AnimationPose::AnimationPose() { m_reorder = false; }

AnimationPose::AnimationPose(const AnimationClip& clip, float ratio, const AnimSkelMap& animSkelMap)
{
    m_bone_indexs = animSkelMap.convert;
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
AnimationPose::AnimationPose(const AnimationClip&   clip,
                             const BoneBlendWeight& weight,
                             float                  ratio,
                             const AnimSkelMap&     animSkelMap)
{
    m_weight      = weight;
    m_bone_indexs = animSkelMap.convert;
    m_reorder     = true;
    extractFromClip(m_bone_poses, clip, ratio);
}

void AnimationPose::extractFromClip(std::vector<Transform>& bones, const AnimationClip& clip, float ratio)
{
    bones.resize(clip.node_count);

    float exact_frame        = ratio * (clip.total_frame - 1);
    int   current_frame_low  = floor(exact_frame); 
    int   current_frame_high = ceil(exact_frame);
    float lerp_ratio         = exact_frame - current_frame_low;
    for (int i = 0; i < clip.node_count; i++)
    {
        const AnimationChannel& channel = clip.node_channels[i];
        bones[i].m_position = Vector3::lerp(
            channel.position_keys[current_frame_low], channel.position_keys[current_frame_high], lerp_ratio);
        bones[i].m_scale    = Vector3::lerp(
            channel.scaling_keys[current_frame_low], channel.scaling_keys[current_frame_high], lerp_ratio);
        bones[i].m_rotation = Quaternion::nLerp(
            lerp_ratio, channel.rotation_keys[current_frame_low], channel.rotation_keys[current_frame_high], true);
    }
}


void AnimationPose::blend(const AnimationPose& pose)
{
    for (int i = 0; i < m_bone_poses.size(); i++)
    {
        auto&       bone_trans_one = m_bone_poses[i];
        const auto& bone_trans_two = pose.m_bone_poses[i];

        // float sum_weight =
        // if (sum_weight != 0)
        {
            // float cur_weight =
            // m_weight.m_blend_weight[i] =
            // bone_trans_one.m_position  =
            // bone_trans_one.m_scale     =
            // bone_trans_one.m_rotation  =
        }
    }
}



