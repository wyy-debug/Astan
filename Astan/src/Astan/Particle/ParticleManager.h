#pragma once


#include <memory>
#include <Astan/Particle/ParticleDesc.h>
#include <Astan/Particle/ParticleCommon.h>

namespace Astan
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

        void createParticleEmitter(const ParticleComponentRes& particle_res,
            ParticleEmitterTransformDesc& transform_desc);

    private:
        GlobalParticleRes m_global_particle_res;
    };
} // namespace Piccolo