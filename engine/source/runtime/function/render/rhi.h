#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

namespace Pilot
{
    class WindowSystem;

    struct RHIInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;
    };

    class RHI
    {
    public:
        virtual ~RHI() = 0;

        virtual void initialize(RHIInitInfo initialize_info) = 0;
        virtual void prepareContext()                        = 0;
        bool         isValidationLayerEnabled() const { return m_enable_validation_Layers; }
        bool         isDebugLabelEnabled() const { return m_enable_debug_utils_label; }
        bool         isPointLightShadowEnabled() const { return m_enable_point_light_shadow; }

    protected:
        bool m_enable_validation_Layers {true};
        bool m_enable_debug_utils_label {true};
        bool m_enable_point_light_shadow {true};

        // used in descriptor pool creation
        uint32_t m_max_vertex_blending_mesh_count {256};
        uint32_t m_max_material_count {256};

    private:
    };

    inline RHI::~RHI() = default;
} // namespace Pilot
