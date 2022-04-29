#include "runtime/function/render/include/render/vulkan_manager/vulkan_common.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_context.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_render_pass.h"

namespace Pilot
{
    class PPointLightShadowPass : public PRenderPassBase
    {
    public:
        void initialize();
        void postInitialize();
        void draw();

        VkDescriptorSetLayout                           _per_mesh_layout;
        MeshPointLightShadowPerframeStorageBufferObject _mesh_point_light_shadow_perframe_storage_buffer_object;

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
