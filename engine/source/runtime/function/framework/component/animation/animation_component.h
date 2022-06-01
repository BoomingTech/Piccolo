#pragma once

#include "runtime/function/animation/skeleton.h"
#include "runtime/function/animation/pose.h"
#include "runtime/function/framework/component/component.h"
#include "runtime/resource/res_type/components/animation.h"
#include "runtime/function/animation/animation_FSM.h"
#include "json11.hpp"
namespace Pilot
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
        void                   animateBasicClip(float ratio, BasicClip* basic_clip);
        void                   blend(float desired_ratio, BlendState* blend_state);
        void                   blend1D(float desired_ratio, BlendSpace1D* blend_state);
        template<typename T>
        void updateSignal(const std::string& key, const T& value)
        {
            m_signal[key] = value;
        }

    protected:
        META(Enable)
        AnimationComponentRes m_animation_res;

        Skeleton m_skeleton;
        AnimationResult       m_animation_result;
        AnimationFSM          m_animation_fsm;
        json11::Json::object  m_signal;
        float                 m_ratio {0};
    };
} // namespace Pilot
