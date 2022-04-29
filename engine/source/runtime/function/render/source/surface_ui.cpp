#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#include "runtime/function/render/include/render/surface.h"
#include <backends/imgui_impl_glfw.h>

namespace Pilot
{
    void surface_ui_register_input(void* surface_ui) { static_cast<SurfaceUI*>(surface_ui)->registerInput(); }

    void surface_ui_on_tick(void* surface_ui, void* ui_state)
    {
        static_cast<SurfaceUI*>(surface_ui)->m_tmp_uistate = static_cast<UIState*>(ui_state);
        static_cast<SurfaceUI*>(surface_ui)->onTick(static_cast<UIState*>(ui_state));
    }

    float surface_ui_content_scale(void* surface_ui) { return static_cast<SurfaceUI*>(surface_ui)->getContentScale(); }

    void SurfaceUI::initialize(std::shared_ptr<SurfaceIO> pio) { m_io = pio; }

    float SurfaceUI::getContentScale() const
    {
        float x_scale, y_scale;
        glfwGetWindowContentScale(m_io->m_window, &x_scale, &y_scale);
        return fmaxf(1.0f, fmaxf(x_scale, y_scale));
    }

    float SurfaceUI::getIndentScale() const
    {
#if defined(__GNUC__) && defined(__MACH__)
        return 1.0f;
#else // Not tested on Linux
        return getContentScale();
#endif
    }

} // namespace Pilot
