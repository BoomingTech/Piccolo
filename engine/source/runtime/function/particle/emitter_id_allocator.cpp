#include "runtime/function/particle/emitter_id_allocator.h"

#include "core/base/macro.h"

namespace Piccolo
{
    std::atomic<ParticleEmitterID> ParticleEmitterIDAllocator::m_next_id {0};

    ParticleEmitterID ParticleEmitterIDAllocator::alloc()
    {
        std::atomic<ParticleEmitterID> new_emitter_ret = m_next_id.load();
        m_next_id++;
        if (m_next_id >= k_invalid_particke_emmiter_id)
        {
            LOG_FATAL("particle emitter id overflow");
        }

        return new_emitter_ret;
    }

    void ParticleEmitterIDAllocator::reset()
    {
        m_next_id.store(0);
    }
} // namespace Piccolo
