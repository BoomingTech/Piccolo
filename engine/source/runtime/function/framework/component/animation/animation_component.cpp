#include "runtime/function/framework/component/animation/animation_component.h"

#include "runtime/function/animation/animation_system.h"
#include "runtime/function/framework/object/object.h"

namespace Piccolo
{
    void AnimationComponent::postLoadResource(std::weak_ptr<GObject> parent_object)
    {
        m_parent_object = parent_object;

        auto skeleton_res = AnimationManager::tryLoadSkeleton(m_animation_res.skeleton_file_path);

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

    const Skeleton& AnimationComponent::getSkeleton() const { return m_skeleton; }
} // namespace Piccolo
