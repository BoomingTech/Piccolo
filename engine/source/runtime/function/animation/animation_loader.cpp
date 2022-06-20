
#include "runtime/function/animation/animation_loader.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/res_type/data/animation_clip.h"
#include "runtime/resource/res_type/data/skeleton_mask.h"

#include "runtime/function/animation/utilities.h"
#include "runtime/function/global/global_context.h"

#include "_generated/serializer/all_serializer.h"

namespace Piccolo
{
    namespace
    {

        // Serial
        std::shared_ptr<RawBone> createBone(SkeletonData* skeleton_data, int parent_index)
        {
            RawBone raw_bone;
            raw_bone.index                    = skeleton_data->bones_map.size();
            raw_bone.parent_index             = parent_index;
            std::shared_ptr<RawBone> bone_ptr = std::make_shared<Piccolo::RawBone>();
            *bone_ptr                         = raw_bone;
            skeleton_data->bones_map.push_back(*bone_ptr);
            return bone_ptr;
        }

        AnimSkelMap buildAnimSkelMap(const AnimNodeMap& anim, const SkeletonData& skeleton)
        {
            AnimSkelMap anim_skel_map;
            for (std::string iter : anim.convert)
            {
                anim_skel_map.convert.push_back(find_index_by_name(skeleton, iter));
            }
            return anim_skel_map;
        }

        void addBoneBind(MeshData& mesh_bind, size_t bone_index, size_t vertex_index, float weight)
        {
            bone_index++;
            // 			if (!mesh_bind.bind[vertex_index])
            // 				mesh_bind.bind[vertex_index] = std::make_shared<SkeletonBinding>();
            SkeletonBinding& vertex_binding = mesh_bind.bind[vertex_index];
            if (vertex_binding.index0 == 0)
            {
                vertex_binding.index0  = bone_index;
                vertex_binding.weight0 = weight;
                return;
            }
            if (vertex_binding.index1 == 0)
            {
                vertex_binding.index1  = bone_index;
                vertex_binding.weight1 = weight;
                return;
            }
            if (vertex_binding.index2 == 0)
            {
                vertex_binding.index2  = bone_index;
                vertex_binding.weight2 = weight;
                return;
            }
            if (vertex_binding.index3 == 0)
            {
                vertex_binding.index3  = bone_index;
                vertex_binding.weight3 = weight;
                return;
            }
            // LOG_ERROR
        }
    } // namespace

    std::shared_ptr<Piccolo::AnimationClip> AnimationLoader::loadAnimationClipData(std::string animation_clip_url)
    {
        AnimationAsset animation_clip;
        g_runtime_global_context.m_asset_manager->loadAsset(animation_clip_url, animation_clip);
        return std::make_shared<Piccolo::AnimationClip>(animation_clip.clip_data);
    }

    std::shared_ptr<Piccolo::SkeletonData> AnimationLoader::loadSkeletonData(std::string skeleton_data_url)
    {
        SkeletonData data;
        g_runtime_global_context.m_asset_manager->loadAsset(skeleton_data_url, data);
        return std::make_shared<Piccolo::SkeletonData>(data);
    }

    std::shared_ptr<Piccolo::AnimSkelMap> AnimationLoader::loadAnimSkelMap(std::string anim_skel_map_url)
    {
        AnimSkelMap data;
        g_runtime_global_context.m_asset_manager->loadAsset(anim_skel_map_url, data);
        return std::make_shared<Piccolo::AnimSkelMap>(data);
    }

    std::shared_ptr<Piccolo::BoneBlendMask> AnimationLoader::loadSkeletonMask(std::string skeleton_mask_file_url)
    {
        BoneBlendMask data;
        g_runtime_global_context.m_asset_manager->loadAsset(skeleton_mask_file_url, data);
        return std::make_shared<Piccolo::BoneBlendMask>(data);
    }

} // namespace Piccolo
