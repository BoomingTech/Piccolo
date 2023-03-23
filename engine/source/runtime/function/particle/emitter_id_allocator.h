#pragma once

#include <atomic>
#include <limits>

namespace Piccolo
{
    using ParticleEmitterID = std::size_t;

    constexpr ParticleEmitterID k_invalid_particke_emmiter_id = std::numeric_limits<std::size_t>::max();

    class ParticleEmitterIDAllocator
    {
    public:
        static ParticleEmitterID alloc();
        static void reset();

    private:
        static std::atomic<ParticleEmitterID> m_next_id;
    };
} // namespace Piccolo
