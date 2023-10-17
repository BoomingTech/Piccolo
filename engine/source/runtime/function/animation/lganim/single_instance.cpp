#include "single_instance.h"

#include "function/animation/animation_system.h"

namespace Piccolo
{
    CSingleAnimInstance::CSingleAnimInstance(AnimationComponentRes* res)
	{
        m_animation_res = res;
		std::shared_ptr<SkeletonData> skeleton_res = AnimationManager::tryLoadSkeleton(m_animation_res->skeleton_file_path);
		m_skeleton.buildSkeleton(*skeleton_res);
	}


	void CSingleAnimInstance::TickAnimation(float delta_time)
	{
        m_animation_res->blend_state.blend_ratio[0] +=
            (delta_time / m_animation_res->blend_state.blend_clip_file_length[0]);
        m_animation_res->blend_state.blend_ratio[0] -= floor(m_animation_res->blend_state.blend_ratio[0]);

        m_skeleton.applyAnimation(AnimationManager::getBlendStateWithClipData(m_animation_res->blend_state));
        m_animation_res->animation_result = m_skeleton.outputAnimationResult();
	}

	const AnimationResult& CSingleAnimInstance::GetResult()
	{
		return m_animation_res->animation_result;
	}
}
