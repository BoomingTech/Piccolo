#pragma once
#include <vector>

#include "anim_instance.h"
#include "runtime/function/animation/skeleton.h"

namespace Piccolo
{
    REFLECTION_TYPE(CSingleAnimInstance)

    CLASS(CSingleAnimInstance : public CAnimInstanceBase, WhiteListFields)
    {
        REFLECTION_BODY(CSingleAnimInstance)
    public:
        CSingleAnimInstance()=default;
        CSingleAnimInstance(AnimationComponentRes* res);

        void TickAnimation(float delta_time) override;

        Skeleton* GetSkeleton() override { return &m_skeleton; }

        const AnimationResult& GetResult() override;

    private:
        AnimationComponentRes* m_animation_res;
        Skeleton m_skeleton;

    };

}
