#pragma once

#include "runtime/function/particle/particle_desc.h"

#include "runtime/resource/res_type/components/emitter.h"
#include "runtime/resource/res_type/global/global_particle.h"

#include "runtime/core/math/matrix4.h"
#include "runtime/core/math/vector4.h"

#include <memory>

namespace Piccolo
{
    class ParticlePass;
    class ParticleManager
    {
    public:
        ParticleManager() = default;

        ~ParticleManager() {};

        void initialize();
        void clear();

        void setParticlePass(ParticlePass* particle_pass);

        const GlobalParticleRes& getGlobalParticleRes();

        void createParticleEmitter(const ParticleComponentRes&   particle_res,
                                   ParticleEmitterTransformDesc& transform_desc);

    private:
        GlobalParticleRes m_global_particle_res;
    };
} // namespace Piccolo