#include "runtime/function/animation/animation_system.h"

#include "resource/res_type/data/skeleton_mask.h"

#include "runtime/function/animation/animation_loader.h"
#include "runtime/function/animation/skeleton.h"

namespace Piccolo
{
    std::map<std::string, std::shared_ptr<SkeletonData>>  AnimationManager::m_skeleton_definition_cache;
    std::map<std::string, std::shared_ptr<AnimationClip>> AnimationManager::m_animation_data_cache;
    std::map<std::string, std::shared_ptr<AnimSkelMap>>   AnimationManager::m_animation_skeleton_map_cache;
    std::map<std::string, std::shared_ptr<BoneBlendMask>> AnimationManager::m_skeleton_mask_cache;

    std::shared_ptr<SkeletonData> AnimationManager::tryLoadSkeleton(std::string file_path)
    {
        std::shared_ptr<SkeletonData> res;
        AnimationLoader               loader;
        auto                          found = m_skeleton_definition_cache.find(file_path);
        if (found == m_skeleton_definition_cache.end())
        {
            res = loader.loadSkeletonData(file_path);
            m_skeleton_definition_cache.emplace(file_path, res);
        }
        else
        {
            res = found->second;
        }
        return res;
    }

    std::shared_ptr<AnimationClip> AnimationManager::tryLoadAnimation(std::string file_path)
    {
        std::shared_ptr<AnimationClip> res;
        AnimationLoader                loader;
        auto                           found = m_animation_data_cache.find(file_path);
        if (found == m_animation_data_cache.end())
        {
            res = loader.loadAnimationClipData(file_path);
            m_animation_data_cache.emplace(file_path, res);
        }
        else
        {
            res = found->second;
        }
        return res;
    }

    std::shared_ptr<AnimSkelMap> AnimationManager::tryLoadAnimationSkeletonMap(std::string file_path)
    {
        std::shared_ptr<AnimSkelMap> res;
        AnimationLoader              loader;
        auto                         found = m_animation_skeleton_map_cache.find(file_path);
        if (found == m_animation_skeleton_map_cache.end())
        {
            res = loader.loadAnimSkelMap(file_path);
            m_animation_skeleton_map_cache.emplace(file_path, res);
        }
        else
        {
            res = found->second;
        }
        return res;
    }

    std::shared_ptr<BoneBlendMask> AnimationManager::tryLoadSkeletonMask(std::string file_path)
    {
        std::shared_ptr<BoneBlendMask> res;
        AnimationLoader                loader;
        auto                           found = m_skeleton_mask_cache.find(file_path);
        if (found == m_skeleton_mask_cache.end())
        {
            res = loader.loadSkeletonMask(file_path);
            m_skeleton_mask_cache.emplace(file_path, res);
        }
        else
        {
            res = found->second;
        }
        return res;
    }

    BlendStateWithClipData AnimationManager::getBlendStateWithClipData(const BlendState& blend_state)
    {

        for (auto animation_file_path : blend_state.blend_clip_file_path)
        {
            tryLoadAnimation(animation_file_path);
        }
        for (auto anim_skel_map_path : blend_state.blend_anim_skel_map_path)
        {
            tryLoadAnimationSkeletonMap(anim_skel_map_path);
        }
        for (auto skeleton_mask_path : blend_state.blend_mask_file_path)
        {
            tryLoadSkeletonMask(skeleton_mask_path);
        }

        BlendStateWithClipData blend_state_with_clip_data;
        blend_state_with_clip_data.clip_count  = blend_state.clip_count;
        blend_state_with_clip_data.blend_ratio = blend_state.blend_ratio;
        for (const auto& iter : blend_state.blend_clip_file_path)
        {
            blend_state_with_clip_data.blend_clip.push_back(*m_animation_data_cache[iter]);
        }
        for (const auto& iter : blend_state.blend_anim_skel_map_path)
        {
            blend_state_with_clip_data.blend_anim_skel_map.push_back(*m_animation_skeleton_map_cache[iter]);
        }
        std::vector<std::shared_ptr<BoneBlendMask>> blend_masks;
        for (auto& iter : blend_state.blend_mask_file_path)
        {
            blend_masks.push_back(m_skeleton_mask_cache[iter]);
            tryLoadAnimationSkeletonMap(m_skeleton_mask_cache[iter]->skeleton_file_path);
        }
        size_t skeleton_bone_count = m_skeleton_definition_cache[blend_masks[0]->skeleton_file_path]->bones_map.size();
        blend_state_with_clip_data.blend_weight.resize(blend_state.clip_count);
        for (size_t clip_index = 0; clip_index < blend_state.clip_count; clip_index++)
        {
            blend_state_with_clip_data.blend_weight[clip_index].blend_weight.resize(skeleton_bone_count);
        }
        for (size_t bone_index = 0; bone_index < skeleton_bone_count; bone_index++)
        {
            float sum_weight = 0;
            for (size_t clip_index = 0; clip_index < blend_state.clip_count; clip_index++)
            {
                if (blend_masks[clip_index]->enabled[bone_index])
                {
                    sum_weight += blend_state.blend_weight[clip_index];
                }
            }
            if (fabs(sum_weight) < 0.0001f)
            {
                // LOG_ERROR
            }
            for (size_t clip_index = 0; clip_index < blend_state.clip_count; clip_index++)
            {
                if (blend_masks[clip_index]->enabled[bone_index])
                {

                    blend_state_with_clip_data.blend_weight[clip_index].blend_weight[bone_index] =
                        blend_state.blend_weight[clip_index] / sum_weight;
                }
                else
                {
                    blend_state_with_clip_data.blend_weight[clip_index].blend_weight[bone_index] = 0;
                }
            }
        }
        return blend_state_with_clip_data;
    }
} // namespace Piccolo
