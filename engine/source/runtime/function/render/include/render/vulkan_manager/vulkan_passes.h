#include "runtime/function/render/include/render/vulkan_manager/vulkan_common.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_context.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_directional_light_pass.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_pick_pass.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_point_light_pass.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_postprocess_pass.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_render_pass.h"

namespace Pilot
{
    struct PLightPassHelperInfo
    {
        VkImageView point_light_shadow_color_image_view;
        VkImageView directional_light_shadow_color_image_view;
    };

    class PLightingPass : public PRenderPassBase
    {
    public:
        // 1: per mesh layout
        // 2: global layout
        // 3: per material layout
        // 4: sky box layout
        // 5: axis layout
        // 6: billboard type particle layout
        enum LayoutType : uint8_t
        {
            _per_mesh = 0,
            _global,
            _material,
            _skybox,
            _axis,
            _particle,
            _layout_type_count
        };

        // 1. model
        // 2. sky box
        // 3. axis
        // 4. billboard type particle
        enum RenderPipeLineType : uint8_t
        {
            _render_pipeline_type_model = 0,
            _render_pipeline_type_skybox,
            _render_pipeline_type_axis,
            _render_pipeline_type_particle,
            _render_pipeline_type_count
        };

        void initialize();
        void draw();

        void setHelperInfo(const PLightPassHelperInfo& helper_info);

        bool                                         m_is_show_axis;
        size_t                                       m_selected_axis;
        MeshPerframeStorageBufferObject              m_mesh_perframe_storage_buffer_object;
        AxisStorageBufferObject                      m_axis_storage_buffer_object;
        ParticleBillboardPerframeStorageBufferObject m_particlebillboard_perframe_storage_buffer_object;

    private:
        void setupRenderPass();
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();

        void setupModelGlobalDescriptorSet();
        void setupSkyboxDescriptorSet();
        void setupAxisDescriptorSet();
        void setupParticleDescriptorSet();

        void drawModel();
        void drawSkybox();
        void drawAxis();
        void drawBillboardParticle();

    private:
        VkImageView m_point_light_shadow_color_image_view;
        VkImageView m_directional_light_shadow_color_image_view;
    };
} // namespace Pilot
