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

        Skeleton skeleton;
        META(Enable)
        AnimationComponentRes animation_component;

    public:
        AnimationComponent() = default;
        AnimationComponent(const AnimationComponentRes& rigidbody_ast, GObject* parent_object);

        void                   tick(float delta_time) override;
        void                   destroy() override {}
        const AnimationResult& getResult() const;
    };
} // namespace Pilot
