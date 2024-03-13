#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Astan
{
    class CameraPose
    {
    public:
        glm::vec3 m_position;
        glm::vec3 m_target;
        glm::vec3 m_up;
    };

    class CameraConfig
    {
    public:
        CameraPose m_pose;
        glm::vec2    m_aspect;
        float      m_z_far;
        float      m_z_near;
    };
    class Color
    {
    public:
        float r;
        float g;
        float b;
        glm::vec3 toVector3() const { return glm::vec3(r, g, b); }

    };

    class SkyBoxIrradianceMap
    {
    public:
        std::string m_negative_x_map;
        std::string m_positive_x_map;
        std::string m_negative_y_map;
        std::string m_positive_y_map;
        std::string m_negative_z_map;
        std::string m_positive_z_map;
    };

    class SkyBoxSpecularMap
    {
    public:
        std::string m_negative_x_map;
        std::string m_positive_x_map;
        std::string m_negative_y_map;
        std::string m_positive_y_map;
        std::string m_negative_z_map;
        std::string m_positive_z_map;
    };

    class DirectionalLight
    {
    public:
        glm::vec3 m_direction;
        Color   m_color;
    };

    class GlobalRenderingRes
    {
    public:
        bool                m_enable_fxaa{ false };
        SkyBoxIrradianceMap m_skybox_irradiance_map;
        SkyBoxSpecularMap   m_skybox_specular_map;
        std::string         m_brdf_map;
        std::string         m_color_grading_map;

        Color            m_sky_color;
        Color            m_ambient_light;
        CameraConfig     m_camera_config;
        DirectionalLight m_directional_light;
    };
} // namespace Piccolo