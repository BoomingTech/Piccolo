#include "runtime/function/render/include/render/vulkan_manager/vulkan_common.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_context.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_directional_light_pass.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_pick_pass.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_point_light_pass.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_render_pass.h"

namespace Pilot
{
    struct PLightPassHelperInfo
    {
        VkImageView point_light_shadow_color_image_view;
        VkImageView directional_light_shadow_color_image_view;
    };

	class PPixelPass : public PRenderPassBase
    {
    public:
        void initialize(VkRenderPass render_pass, VkImageView input_attachment);
        void draw();

        void updateAfterFramebufferRecreate(VkImageView input_attachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };

    class PColorGradingPass : public PRenderPassBase
    {
    public:
        void initialize(VkRenderPass render_pass, VkImageView input_attachment);
        void draw();

        void updateAfterFramebufferRecreate(VkImageView input_attachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };

    class PToneMappingPass : public PRenderPassBase
    {
    public:
        void initialize(VkRenderPass render_pass, VkImageView input_attachment);
        void draw();

        void updateAfterFramebufferRecreate(VkImageView input_attachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };

    class PUIPass : public PRenderPassBase
    {
    public:
        void initialize(VkRenderPass render_pass);
        void setSurfaceUI(void* surface_ui);
        void draw(void* ui_state);

    private:
        void  setDefaultStyle();
        void  fontsUpload();
        void* m_surface_ui;
    };

    class PCombineUIPass : public PRenderPassBase
    {
    public:
        void initialize(VkRenderPass render_pass, VkImageView scene_input_attachment, VkImageView ui_input_attachment);
        void draw();

        void updateAfterFramebufferRecreate(VkImageView scene_input_attachment, VkImageView ui_input_attachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };

    extern void  surface_ui_register_input(void* m_surface_ui);
    extern void  surface_ui_on_tick(void* surface_ui, void* ui_state);
    extern float surface_ui_content_scale(void* surface_ui);

    enum
    {
        _main_camera_pass_gbuffer_a               = 0,
        _main_camera_pass_gbuffer_b               = 1,
        _main_camera_pass_gbuffer_c               = 2,
        _main_camera_pass_backup_buffer_odd       = 3,
        _main_camera_pass_backup_buffer_even      = 4,
        _main_camera_pass_depth                   = 5,
        _main_camera_pass_swap_chain_image        = 6,
        _main_camera_pass_custom_attachment_count = 5,
        _main_camera_pass_attachment_count        = 7,
    };

    enum
    {
        _main_camera_subpass_basepass = 0,
        _main_camera_subpass_deferred_lighting,
        _main_camera_subpass_forward_lighting,
        _main_camera_subpass_tone_mapping,
        _main_camera_subpass_color_grading,
    	_main_camera_subpass_pixel,
        _main_camera_subpass_ui,
        _main_camera_subpass_combine_ui,
        _main_camera_subpass_count
    };

    class PMainCameraPass : public PRenderPassBase
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

        void initialize();

        void draw(PColorGradingPass& color_grading_pass,
                  PToneMappingPass&  tone_mapping_pass,
				  PPixelPass&        pixel_pass,
                  PUIPass&           ui_pass,
                  PCombineUIPass&    combine_ui_pass,
                  uint32_t           current_swapchain_image_index,
                  void*              ui_state);

        // legacy
        void drawForward(PColorGradingPass& color_grading_pass,
                         PToneMappingPass&  tone_mapping_pass,
						 PPixelPass&        pixel_pass,
                         PUIPass&           ui_pass,
                         PCombineUIPass&    combine_ui_pass,
                         uint32_t           current_swapchain_image_index,
                         void*              ui_state);

        void setHelperInfo(const PLightPassHelperInfo& helper_info);

        bool                                         m_is_show_axis;
        size_t                                       m_selected_axis;
        MeshPerframeStorageBufferObject              m_mesh_perframe_storage_buffer_object;
        AxisStorageBufferObject                      m_axis_storage_buffer_object;
        ParticleBillboardPerframeStorageBufferObject m_particlebillboard_perframe_storage_buffer_object;

        void updateAfterFramebufferRecreate();

    private:
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
        void drawBillboardParticle();
        void drawAxis();

    private:
        VkImageView                m_point_light_shadow_color_image_view;
        VkImageView                m_directional_light_shadow_color_image_view;
        std::vector<VkFramebuffer> m_swapchain_framebuffers;
    };

} // namespace Pilot
