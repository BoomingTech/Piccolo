#pragma once

#include "runtime/function/render/render_pass.h"

#include "runtime/function/render/passes/color_grading_pass.h"
#include "runtime/function/render/passes/combine_ui_pass.h"
#include "runtime/function/render/passes/fxaa_pass.h"
#include "runtime/function/render/passes/tone_mapping_pass.h"
#include "runtime/function/render/passes/ui_pass.h"
#include "runtime/function/render/passes/particle_pass.h"

namespace Piccolo
{
    class RenderResourceBase;

    struct MainCameraPassInitInfo : RenderPassInitInfo
    {
        bool enble_fxaa;
    };

    class MainCameraPass : public RenderPass
    {
    public:
        // 1: per mesh layout
        // 2: global layout
        // 3: mesh per material layout
        // 4: sky box layout
        // 5: axis layout
        // 6: billboard type particle layout
        // 7: gbuffer lighting
        enum LayoutType : uint8_t
        {
            _per_mesh = 0,
            _mesh_global,
            _mesh_per_material,
            _skybox,
            _axis,
            _particle,
            _deferred_lighting,
            _layout_type_count
        };

        // 1. model
        // 2. sky box
        // 3. axis
        // 4. billboard type particle
        enum RenderPipeLineType : uint8_t
        {
            _render_pipeline_type_mesh_gbuffer = 0,
            _render_pipeline_type_deferred_lighting,
            _render_pipeline_type_mesh_lighting,
            _render_pipeline_type_skybox,
            _render_pipeline_type_axis,
            _render_pipeline_type_particle,
            _render_pipeline_type_count
        };

        void initialize(const RenderPassInitInfo* init_info) override final;

        void preparePassData(std::shared_ptr<RenderResourceBase> render_resource) override final;

        void draw(ColorGradingPass& color_grading_pass,
            FXAAPass& fxaa_pass,
            ToneMappingPass& tone_mapping_pass,
            UIPass& ui_pass,
            CombineUIPass& combine_ui_pass,
            ParticlePass& particle_pass,
            uint32_t          current_swapchain_image_index);

        void drawForward(ColorGradingPass& color_grading_pass,
            FXAAPass& fxaa_pass,
            ToneMappingPass& tone_mapping_pass,
            UIPass& ui_pass,
            CombineUIPass& combine_ui_pass,
            ParticlePass& particle_pass,
            uint32_t          current_swapchain_image_index);

        void copyNormalAndDepthImage();

        RHIImageView* m_point_light_shadow_color_image_view;
        RHIImageView* m_directional_light_shadow_color_image_view;

        bool                                         m_is_show_axis{ false };
        bool                                         m_enable_fxaa{ false };
        size_t                                       m_selected_axis{ 3 };
        MeshPerframeStorageBufferObject              m_mesh_perframe_storage_buffer_object;
        AxisStorageBufferObject                      m_axis_storage_buffer_object;

        void updateAfterFramebufferRecreate();

        RHICommandBuffer* getRenderCommandBuffer();

        void setParticlePass(std::shared_ptr<ParticlePass> pass);

    private:
        void setupParticlePass();
        void setupAttachments();
        void setupRenderPass();
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
        void setupFramebufferDescriptorSet();
        void setupSwapchainFramebuffers();

        void setupModelGlobalDescriptorSet();
        void setupSkyboxDescriptorSet();
        void setupAxisDescriptorSet();
        void setupParticleDescriptorSet();
        void setupGbufferLightingDescriptorSet();

        void drawMeshGbuffer();
        void drawDeferredLighting();
        void drawMeshLighting();
        void drawSkybox();
        void drawAxis();



    private:
        std::vector<RHIFramebuffer*> m_swapchain_framebuffers;
        std::shared_ptr<ParticlePass> m_particle_pass;
    };
} // namespace Piccolo
