#pragma once

#include "runtime/function/render/render_pipeline_base.h"

namespace Piccolo
{
    class RenderPipeline : public RenderPipelineBase
    {
    public:
        virtual void initialize(RenderPipelineInitInfo init_info) override final;

        virtual void forwardRender(std::shared_ptr<RHI>                rhi,
                                   std::shared_ptr<RenderResourceBase> render_resource) override final;

        virtual void deferredRender(std::shared_ptr<RHI>                rhi,
                                    std::shared_ptr<RenderResourceBase> render_resource) override final;

        void passUpdateAfterRecreateSwapchain();

        virtual uint32_t getGuidOfPickedMesh(const Vector2& picked_uv) override final;

        void setAxisVisibleState(bool state);

        void setSelectedAxis(size_t selected_axis);
    };
} // namespace Piccolo
