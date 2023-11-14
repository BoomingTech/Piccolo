#pragma once

#include "runtime/function/animation/skeleton.h"
#include "runtime/function/framework/component/component.h"
#include "runtime/resource/res_type/components/animation.h"

#include "runtime/function/animation/lganim/anim_instance.h"

namespace Piccolo
{
    REFLECTION_TYPE(AnimationComponent)
    CLASS(AnimationComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(AnimationComponent)

    public:
        AnimationComponent() = default;

        void postLoadResource(std::weak_ptr<GObject> parent_object) override;

        void tick(float delta_time) override;

        const AnimationResult& getResult() const;

        const Skeleton& getSkeleton() const;
        const std::vector<Matrix4x4>& GetComponentResult() const;
        const std::vector<uint32_t>&  GetParentInfo() const;


        bool HasRootMotion() const;
        Transform GetRootMotion() const;

        CAnimInstanceBase* getInstance() const
        {
	        return m_anim_instance.get();
        }
    protected:
        META(Enable)
        AnimationComponentRes              m_animation_res;
		
        std::shared_ptr<CAnimInstanceBase> m_anim_instance;

        Skeleton* m_skeleton;

        int32_t m_ticked_frame;
    };
} // namespace Piccolo
