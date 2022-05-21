#pragma once

#include "runtime/function/render/render_pass.h"

namespace Pilot
{
    struct ToneMappingPassInitInfo : RenderPassInitInfo
    {
        VkRenderPass render_pass;
        VkImageView  input_attachment;
    };

    class ToneMappingPass : public RenderPass
    {
    public:
        virtual void initialize(const RenderPassInitInfo* init_info) override final;
        void         draw();

        void updateAfterFramebufferRecreate(VkImageView input_attachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };
} // namespace Pilot
