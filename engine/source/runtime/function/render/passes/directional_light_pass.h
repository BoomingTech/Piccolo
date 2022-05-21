#pragma once

#include "runtime/function/render/render_pass.h"

namespace Pilot
{
    class RenderResourceBase;

    class DirectionalLightShadowPass : public RenderPass
    {
    public:
        virtual void initialize(const RenderPassInitInfo* init_info) override final;
        virtual void postInitialize() override final;
        virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource) override final;
        void         draw();

        VkDescriptorSetLayout _per_mesh_layout;
        MeshDirectionalLightShadowPerframeStorageBufferObject
            _mesh_directional_light_shadow_perframe_storage_buffer_object;

    private:
        void setupAttachments();
        void setupRenderPass();
        void setupFramebuffer();
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
        void drawModel();
    };
} // namespace Pilot
