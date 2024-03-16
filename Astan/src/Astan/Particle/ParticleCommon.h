#pragma once

#include <glm/glm.hpp>
#include <limits>

namespace Astan
{
    // max particle pool size
    static constexpr int   s_max_particles{ 300000 };
    static constexpr int   s_default_particle_emit_gap{ 10 };
    static constexpr int   s_default_particle_emit_count{ 100000 };
    static constexpr int   s_default_particle_life_time{ 10 };
    static constexpr float s_default_particle_time_step{ 0.004 };

    static const glm::vec4 s_default_emiter_position{ 5.71, 13.53, 3.0, 0.5 };
    static const glm::vec4 s_default_emiter_velocity{ 0.02, 0.02, 2.5, 4.0 };
    static const glm::vec4 s_default_emiter_acceleration{ 0.00, 0.00, -2.5, 0.0 };
    static const glm::vec3 s_default_emiter_size{ 0.02, 0.02, 0.0 };
    static const glm::vec2 s_default_emiter_life{ 1.2, 0.0 };

    enum class EMITTER_TYPE
    {
        POINT = 0,
        MESH,
        INVALID
    };


} // namespace Piccolo
