#include "aspch.h"
#include <atomic>
#include <Astan/Particle/EmitterIDAllocator.h>

namespace Astan
{
    std::atomic<ParticleEmitterID> ParticleEmitterIDAllocator::m_next_id{ 0 };

    ParticleEmitterID ParticleEmitterIDAllocator::alloc()
    {
        std::atomic<ParticleEmitterID> new_emitter_ret = m_next_id.load();
        m_next_id++;
        if (m_next_id >= k_invalid_particke_emmiter_id)
        {
            AS_CORE_INFO("particle emitter id overflow");
        }

        return new_emitter_ret;
    }

    void ParticleEmitterIDAllocator::reset()
    {
        m_next_id.store(0);
    }
} // namespace Piccolo
