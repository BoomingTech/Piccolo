#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include "runtime/core/color/color.h"
#include "runtime/core/math/vector3.h"

#include "runtime/resource/res_type/data/camera_config.h"

namespace Piccolo
{
    REFLECTION_TYPE(SkyBoxIrradianceMap)
    CLASS(SkyBoxIrradianceMap, Fields)
    {
        REFLECTION_BODY(SkyBoxIrradianceMap);

    public:
        std::string m_negative_x_map;
        std::string m_positive_x_map;
        std::string m_negative_y_map;
        std::string m_positive_y_map;
        std::string m_negative_z_map;
        std::string m_positive_z_map;
    };

    REFLECTION_TYPE(SkyBoxSpecularMap)
    CLASS(SkyBoxSpecularMap, Fields)
    {
        REFLECTION_BODY(SkyBoxSpecularMap);

    public:
        std::string m_negative_x_map;
        std::string m_positive_x_map;
        std::string m_negative_y_map;
        std::string m_positive_y_map;
        std::string m_negative_z_map;
        std::string m_positive_z_map;
    };

    REFLECTION_TYPE(DirectionalLight)
    CLASS(DirectionalLight, Fields)
    {
        REFLECTION_BODY(DirectionalLight);

    public:
        Vector3 m_direction;
        Color   m_color;
    };

    REFLECTION_TYPE(GlobalRenderingRes)
    CLASS(GlobalRenderingRes, Fields)
    {
        REFLECTION_BODY(GlobalRenderingRes);

    public:
        bool                m_enable_fxaa {false};
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