#pragma once
#include <Astan/Particle/EmitterIDAllocator.h>

#include <glm/glm.hpp>
#include <string>


namespace Astan
{

    class GlobalParticleRes
    {
        public:
            GlobalParticleRes() {}
            int         m_emit_gap;
            int         m_emit_count;
            float       m_time_step;
            float       m_max_life;
            glm::vec3     m_gravity;
            std::string m_particle_billboard_texture_path;
            std::string m_piccolo_logo_texture_path;
    };
    class ParticleComponentRes
    {
    public:
        glm::vec3    m_local_translation; // local translation
        glm::vec4    m_local_rotation = glm::vec4(1.0f,0.0f,0.0f,0.0f);    // local rotation
        glm::vec4    m_velocity;          // velocity base & variance
        glm::vec4    m_acceleration;      // acceleration base & variance
        glm::vec3    m_size;              // size base & variance
        int        m_emitter_type;
        glm::vec2    m_life;  // life base & variance
        glm::vec4    m_color; // color rgba
    };
    struct ParticleEmitterTransformDesc
    {
        ParticleEmitterID m_id;
        glm::vec4           m_position;
        glm::mat4         m_rotation;
    };

    struct ParticleEmitterDesc
    {
        glm::vec4   m_position;
        glm::mat4 m_rotation;
        glm::vec4   m_velocity;
        glm::vec4   m_acceleration;
        glm::vec3   m_size;
        int       m_emitter_type;
        glm::vec2   m_life;
        glm::vec2   m_padding;
        glm::vec4   m_color;

        ParticleEmitterDesc() = default;

        ParticleEmitterDesc(const ParticleComponentRes& component_res, ParticleEmitterTransformDesc& transform_desc) :
            m_position(transform_desc.m_position), m_rotation(transform_desc.m_rotation),
            m_velocity(component_res.m_velocity), m_acceleration(component_res.m_acceleration),
            m_size(component_res.m_size), m_emitter_type(component_res.m_emitter_type), m_life(component_res.m_life),
            m_color(component_res.m_color)
        {}
    };
} // namespace Piccolo
