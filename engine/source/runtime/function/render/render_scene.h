#pragma once

#include "runtime/function/render/light.h"
#include "runtime/function/render/render_entity.h"

#include <optional>
#include <vector>

namespace Pilot
{
    class RenderScene
    {
    public:
        // light
        PAmbientLight     m_ambient_light;
        PDirectionalLight m_directional_light;
        PPointLightList   m_point_light_list;

        // render entities
        std::vector<RenderEntity> m_render_entities;

        // axis, for editor
        std::optional<RenderEntity> m_render_axis;
    };
} // namespace Pilot
