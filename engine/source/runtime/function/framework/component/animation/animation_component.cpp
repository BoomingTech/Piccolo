#include "runtime/function/framework/component/animation/animation_component.h"

#include "function/framework/world/world_manager.h"
#include "function/global/global_context.h"
#include "runtime/function/framework/object/object.h"

#include "runtime/function/animation/lganim/single_instance.h"
#include "runtime/function/animation/lganim/mm_instance.h"

namespace Piccolo
{
    void AnimationComponent::postLoadResource(std::weak_ptr<GObject> parent_object)
    {
        m_parent_object = parent_object;

        //m_anim_instance = std::make_shared<CSingleAnimInstance>(&m_animation_res);
        m_anim_instance = std::make_shared<CAnimInstanceMotionMatching>(&m_animation_res);
       
        m_skeleton = m_anim_instance->GetSkeleton();
    }

    void AnimationComponent::tick(float delta_time)
    {
        // 保证一帧只更新一次
        const int32_t frame_count = g_runtime_global_context.m_world_manager->getFrameCount();
        if (m_ticked_frame == frame_count)
	    {
            return;
	    }
        m_ticked_frame = frame_count;


    	m_anim_instance->TickAnimation(delta_time);
    }

    const AnimationResult& AnimationComponent::getResult() const { return m_anim_instance->GetResult(); }

    const Skeleton& AnimationComponent::getSkeleton() const { return *m_skeleton; }

    const std::vector<Matrix4x4>& AnimationComponent::GetComponentResult() const
    {
        return m_anim_instance->GetComponentResult();
    }

    const std::vector<uint32_t>& AnimationComponent::GetParentInfo() const
    {
    	return m_anim_instance->GetParentInfo();
    }

    bool AnimationComponent::HasRootMotion() const
    {
	    return m_anim_instance->HasRootMotion();
    }

    Transform AnimationComponent::GetRootMotion() const
    {
	    return m_anim_instance->GetRootMotion();
    }
} // namespace Piccolo
