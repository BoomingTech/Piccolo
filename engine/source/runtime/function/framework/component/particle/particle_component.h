#pragma once

#include "runtime/function/framework/component/component.h"
#include "runtime/function/particle/particle_desc.h"

#include "runtime/resource/res_type/components/emitter.h"

#include "runtime/core/math/matrix4.h"
#include "runtime/core/math/transform.h"

namespace Piccolo
{
    REFLECTION_TYPE(ParticleComponent)
    CLASS(ParticleComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(ParticleComponent)

    public:
        ParticleComponent() {}

        void postLoadResource(std::weak_ptr<GObject> parent_object) override;

        void tick(float delta_time) override;

    private:
        void computeGlobalTransform();

        META(Enable)
        ParticleComponentRes m_particle_res;

        Matrix4x4 m_local_transform;

        ParticleEmitterTransformDesc m_transform_desc;
    };
} // namespace Piccolo