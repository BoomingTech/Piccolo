#pragma once

#include "runtime/function/render/render_pass.h"

namespace Piccolo
{
    struct CombineUIPassInitInfo : RenderPassInitInfo
    {
        RHIRenderPass* render_pass;
        RHIImageView*  scene_input_attachment;
        RHIImageView*  ui_input_attachment;
    };

    class CombineUIPass : public RenderPass
    {
    public:
        void initialize(const RenderPassInitInfo* init_info) override final;
        void draw() override final;

        void updateAfterFramebufferRecreate(RHIImageView* scene_input_attachment, RHIImageView* ui_input_attachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };
} // namespace Piccolo
