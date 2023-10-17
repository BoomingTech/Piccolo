#include "runtime/function/framework/component/animation/animation_component.h"

#include "runtime/function/animation/animation_system.h"
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
    	m_anim_instance->TickAnimation(delta_time);
    }

    const AnimationResult& AnimationComponent::getResult() const { return m_anim_instance->GetResult(); }

    const Skeleton& AnimationComponent::getSkeleton() const { return *m_skeleton; }
} // namespace Piccolo
