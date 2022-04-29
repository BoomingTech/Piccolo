#pragma once

#include "runtime/function/animation/skeleton.h"
#include "runtime/function/framework/component/component.h"
#include "runtime/resource/res_type/components/animation.h"

namespace Pilot
{
    REFLECTION_TYPE(AnimationComponent)
    CLASS(AnimationComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(AnimationComponent)

        Skeleton m_skeleton;
        META(Enable)
        AnimationComponentRes m_animation_res;

    public:
        AnimationComponent() = default;
        AnimationComponent(const AnimationComponentRes& animation_res, GObject* parent_object);

        void tick(float delta_time) override;

        const AnimationResult& getResult() const;
    };
} // namespace Pilot
