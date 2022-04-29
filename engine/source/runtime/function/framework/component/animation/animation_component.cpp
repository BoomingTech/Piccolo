#include "runtime/function/framework/component/animation/animation_component.h"

#include "runtime/function/animation/animation_system.h"
#include "runtime/function/framework/object/object.h"

namespace Pilot
{
    AnimationComponent::AnimationComponent(const AnimationComponentRes& animation_res, GObject* parent_object) :
        Component(parent_object), m_animation_res(animation_res)
    {
        auto skeleton_res = AnimationManager::tryLoadSkeleton(animation_res.skeleton_file_path);

        m_skeleton.buildSkeleton(*skeleton_res);
    }

    void AnimationComponent::tick(float delta_time)
    {
        m_animation_res.blend_state.blend_ratio[0] +=
            (delta_time / m_animation_res.blend_state.blend_clip_file_length[0]);
        m_animation_res.blend_state.blend_ratio[0] -= floor(m_animation_res.blend_state.blend_ratio[0]);

        m_skeleton.applyAnimation(AnimationManager::getBlendStateWithClipData(m_animation_res.blend_state));
        m_animation_res.animation_result = m_skeleton.outputAnimationResult();
    }

    const AnimationResult& AnimationComponent::getResult() const { return m_animation_res.animation_result; }
} // namespace Pilot
