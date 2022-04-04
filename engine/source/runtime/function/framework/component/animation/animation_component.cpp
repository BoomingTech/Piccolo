#include "runtime/function/framework/component/animation/animation_component.h"

#include "runtime/engine.h"
#include "runtime/function/animation/animation_system.h"
#include "runtime/function/framework/object/object.h"

namespace Pilot
{
    AnimationComponent::AnimationComponent(const AnimationComponentRes& rigidbody_ast, GObject* parent_object) :
        Component(parent_object)
    {
        animation_component = rigidbody_ast;
        auto skeleton_res   = AnimationManager::tryLoadSkeleton(animation_component.skeleton_file_path);

        skeleton.buildSkeleton(*skeleton_res);
    }

    void AnimationComponent::tick(float delta_time)
    {
        if ((m_tick_in_editor_mode == false) && g_is_editor_mode)
            return;

        animation_component.blend_state.blend_ratio[0] +=
            (delta_time / animation_component.blend_state.blend_clip_file_length[0]);
        animation_component.blend_state.blend_ratio[0] -= floor(animation_component.blend_state.blend_ratio[0]);

        skeleton.applyAnimation(AnimationManager::getBlendStateWithClipData(animation_component.blend_state));
        animation_component.animation_result = skeleton.outputAnimationResult();
    }

    const AnimationResult& AnimationComponent::getResult() const { return animation_component.animation_result; }
} // namespace Pilot
