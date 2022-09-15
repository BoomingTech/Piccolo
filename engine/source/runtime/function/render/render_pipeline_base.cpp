#include "runtime/function/render/render_pipeline_base.h"

#include "runtime/core/base/macro.h"

namespace Piccolo
{
    void RenderPipelineBase::preparePassData(std::shared_ptr<RenderResourceBase> render_resource)
    {
        m_main_camera_pass->preparePassData(render_resource);
        m_pick_pass->preparePassData(render_resource);
        m_directional_light_pass->preparePassData(render_resource);
        m_point_light_shadow_pass->preparePassData(render_resource);
        m_particle_pass->preparePassData(render_resource);
    }
    void RenderPipelineBase::forwardRender(std::shared_ptr<RHI>                rhi,
                                           std::shared_ptr<RenderResourceBase> render_resource)
    {}
    void RenderPipelineBase::deferredRender(std::shared_ptr<RHI>                rhi,
                                            std::shared_ptr<RenderResourceBase> render_resource)
    {}
    void RenderPipelineBase::initializeUIRenderBackend(WindowUI* window_ui)
    {
        m_ui_pass->initializeUIRenderBackend(window_ui);
    }
} // namespace Piccolo
