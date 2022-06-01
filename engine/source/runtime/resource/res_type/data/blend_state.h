#pragma once
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/resource/res_type/data/animation_clip.h"
#include "runtime/resource/res_type/data/animation_skeleton_node_map.h"
#include <string>
#include <vector>
namespace Pilot
{

    REFLECTION_TYPE(BoneBlendWeight)
    CLASS(BoneBlendWeight, Fields)
    {
        REFLECTION_BODY(BoneBlendWeight);

    public:
        std::vector<float> m_blend_weight;
    };

    REFLECTION_TYPE(ClipData)
    CLASS(ClipData, Fields)
    {
        REFLECTION_BODY(ClipData);

    public:
        AnimationClip   m_clip;
        AnimSkelMap     m_anim_skel_map;
    };

    REFLECTION_TYPE(BlendStateWithClipData)
    CLASS(BlendStateWithClipData, Fields)
    {
        REFLECTION_BODY(BlendStateWithClipData);

    public:
        int                          m_clip_count;
        std::vector<AnimationClip>   m_blend_clip;
        std::vector<AnimSkelMap>     m_blend_anim_skel_map;
        std::vector<BoneBlendWeight> m_blend_weight;
        std::vector<float>           m_blend_ratio;
    };

    REFLECTION_TYPE(ClipBase)
    CLASS(ClipBase, Fields)
    {
        REFLECTION_BODY(ClipBase);

    public:
        std::string              m_name;
        virtual ~ClipBase() = default;
        virtual float getLength() const { return 0; }
    };

    REFLECTION_TYPE(BasicClip)
    CLASS(BasicClip : public ClipBase, Fields)
    {
        REFLECTION_BODY(BasicClip);

    public:
        std::string m_clip_file_path;
        float       m_clip_file_length;
        std::string m_anim_skel_map_path;
        virtual ~BasicClip() override {}
        virtual float getLength() const override
        {
            return m_clip_file_length;
        }
    };

    REFLECTION_TYPE(BlendState)
    CLASS(BlendState : public ClipBase, Fields)
    {
        REFLECTION_BODY(BlendState);

    public:
        int                      m_clip_count;
        std::vector<std::string> m_blend_clip_file_path;
        std::vector<float>       m_blend_clip_file_length;
        std::vector<std::string> m_blend_anim_skel_map_path;
        std::vector<float>       m_blend_weight;
        std::vector<std::string> m_blend_mask_file_path;
        std::vector<float>       m_blend_ratio;
        virtual ~BlendState() override {}
        virtual float getLength() const override
        {
            float length = 0;
            for (int i = 0; i < m_clip_count; i++)
            {
                auto curweight = m_blend_weight[i];
                length += curweight * m_blend_clip_file_length[i];
            }
            return length;
        }
    };

    REFLECTION_TYPE(BlendSpace1D)
    CLASS(BlendSpace1D : public BlendState, Fields)
    {
        REFLECTION_BODY(BlendSpace1D);

    public:
        std::string m_key;
        // enum KeyType
        //{TypeDouble, TypeInt};

        std::vector<double> m_values;

        virtual ~BlendSpace1D()override {}
    };

} // namespace Pilot