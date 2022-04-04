#include "runtime/function/render/include/render/vulkan_manager/vulkan_common.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_context.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_render_pass.h"

namespace Pilot
{
    class PPostprocessPass : public PRenderPassBase
    {
    public:
        void initialize();
        void draw();

        void updateAfterFramebufferRecreate();

        VkDescriptorSetLayout                           _per_mesh_layout;
        MeshPointLightShadowPerframeStorageBufferObject _mesh_point_light_shadow_perframe_storage_buffer_object;

        // post-process pass is the second subpass of the common lighting pass
        VkRenderPass _render_pass;

    private:
        void setupAttachments();
        void setupRenderPass();
        void setupFramebuffer();
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };
} // namespace Pilot
