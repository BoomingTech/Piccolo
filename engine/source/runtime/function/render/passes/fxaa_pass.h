#pragma once

#include "runtime/function/render/render_pass.h"

namespace Piccolo
{
    class WindowUI;

    struct FXAAPassInitInfo : RenderPassInitInfo
    {
        RHIRenderPass* render_pass;
        RHIImageView*  input_attachment;
    };

    class FXAAPass : public RenderPass
    {
    public:
        void initialize(const RenderPassInitInfo* init_info) override final;
        void draw() override final;

        void updateAfterFramebufferRecreate(RHIImageView* input_attachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };
} // namespace Piccolo
