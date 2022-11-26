#include "runtime/function/animation/skeleton.h"

#include "runtime/core/math/math.h"

#include "runtime/function/animation/utilities.h"

namespace Piccolo
{
    Skeleton::~Skeleton() { delete[] m_bones; }

    void Skeleton::resetSkeleton()
    {
        for (size_t i = 0; i < m_bone_count; i++)
            m_bones[i].resetToInitialPose();
    }

    void Skeleton::buildSkeleton(const SkeletonData& skeleton_definition)
    {
        m_is_flat = skeleton_definition.is_flat;
        if (m_bones != nullptr)
        {
            delete[] m_bones;
        }
        if (!m_is_flat || !skeleton_definition.in_topological_order)
        {
            // LOG_ERROR
            return;
        }
        m_bone_count = skeleton_definition.bones_map.size();
        m_bones      = new Bone[m_bone_count];
        for (size_t i = 0; i < m_bone_count; i++)
        {
            const RawBone bone_definition = skeleton_definition.bones_map[i];
            Bone*         parent_bone     = find_by_index(m_bones, bone_definition.parent_index, i, m_is_flat);
            m_bones[i].initialize(std::make_shared<RawBone>(bone_definition), parent_bone);
        }
    }

    void Skeleton::applyAnimation(const BlendStateWithClipData& blend_state)
    {
        if (!m_bones)
        {
            return;
        }
        resetSkeleton();
        for (size_t clip_index = 0; clip_index < 1; clip_index++)
        {
            const AnimationClip& animation_clip = blend_state.blend_clip[clip_index];
            const float          phase          = blend_state.blend_ratio[clip_index];
            const AnimSkelMap&   anim_skel_map  = blend_state.blend_anim_skel_map[clip_index];

            float exact_frame        = phase * (animation_clip.total_frame - 1);
            int   current_frame_low  = floor(exact_frame);
            int   current_frame_high = ceil(exact_frame);
            float lerp_ratio         = exact_frame - current_frame_low;
            // for (size_t node_index = 0; node_index < 0; node_index++)
            for (size_t node_index = 0;
                 node_index < animation_clip.node_count && node_index < anim_skel_map.convert.size();
                 node_index++)
            {
                AnimationChannel channel    = animation_clip.node_channels[node_index];
                size_t           bone_index = anim_skel_map.convert[node_index];
                float            weight     = 1; // blend_state.blend_weight[clip_index]->blend_weight[bone_index];
                weight                      = 1;
                if (fabs(weight) < 0.0001f)
                {
                    continue;
                }
                if (bone_index == std::numeric_limits<size_t>().max())
                {
                    // LOG_WARNING
                    continue;
                }
                Bone* bone = &m_bones[bone_index];
                if (channel.position_keys.size() <= current_frame_high)
                {
                    current_frame_high = channel.position_keys.size() - 1;
                }
                if (channel.scaling_keys.size() <= current_frame_high)
                {
                    current_frame_high = channel.scaling_keys.size() - 1;
                }
                if (channel.rotation_keys.size() <= current_frame_high)
                {
                    current_frame_high = channel.rotation_keys.size() - 1;
                }
                current_frame_low = (current_frame_low < current_frame_high) ? current_frame_low : current_frame_high;
                Vector3 position  = Vector3::lerp(
                    channel.position_keys[current_frame_low], channel.position_keys[current_frame_high], lerp_ratio);
                Vector3 scaling = Vector3::lerp(
                    channel.scaling_keys[current_frame_low], channel.scaling_keys[current_frame_high], lerp_ratio);
                Quaternion rotation = Quaternion::nLerp(lerp_ratio,
                                                        channel.rotation_keys[current_frame_low],
                                                        channel.rotation_keys[current_frame_high],
                                                        true);

                {
                    bone->rotate(rotation);
                    bone->scale(scaling);
                    bone->translate(position);

                    // bone->rotate({ {},0.01,0,0,1 });
                    // bone->scale({ {},0.9,0.9,0.9 });
                }
            }
        }
        // bones[77].rotate(Quaternion{ {},1,0,0,1 });
        // bones[18].translate(Vector3{ {},0,1,0 });
        // bones[0].setScale(Vector3{ {},0,1,0.01 });
        for (size_t i = 0; i < m_bone_count; i++)
        {
            m_bones[i].update();
        }
#ifdef _DEBUG
        // bones[107].m_derived_position += Vector3{ {},10, 0, 0 };
#endif
    }

    AnimationResult Skeleton::outputAnimationResult()
    {
        AnimationResult animation_result;
        for (size_t i = 0; i < m_bone_count; i++)
        {
            std::shared_ptr<AnimationResultElement> animation_result_element =
                std::make_shared<AnimationResultElement>();
            Bone* bone                      = &m_bones[i];
            animation_result_element->index = bone->getID() + 1;
            Vector3 temp_translation        = bone->_getDerivedPosition();

            // TODO: the unit of the joint matrices is wrong
            Vector3 temp_scale = bone->_getDerivedScale();

            Quaternion temp_rotation = bone->_getDerivedOrientation();

            // auto scale = bone->_getDerivedTScale();
            // scale.x = 1.f / scale.x;
            // scale.y = 1.f / scale.y;
            // scale.z = 1.f / scale.z;

            // auto revTmat = getMatrix(
            //	-bone->_getDerivedTPosition(),
            //	scale,
            //	conjugate( bone->_getDerivedTOrientation())
            //);
            auto objMat =
                Transform(bone->_getDerivedPosition(), bone->_getDerivedOrientation(), bone->_getDerivedScale())
                    .getMatrix();

            auto resMat = objMat * bone->_getInverseTpose();

            animation_result_element->transform = resMat.toMatrix4x4_();

            animation_result.node.push_back(*animation_result_element);
        }
        return animation_result;
    }

    const Bone* Skeleton::getBones() const
    {
        return m_bones;
    }

    int32_t Skeleton::getBonesCount() const
    {
        return m_bone_count;
    }
} // namespace Piccolo