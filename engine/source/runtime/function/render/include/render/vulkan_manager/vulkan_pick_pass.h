#include "runtime/function/render/include/render/vulkan_manager/vulkan_common.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_context.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_render_pass.h"

namespace Pilot
{
    class PPickPass : public PRenderPassBase
    {
    public:
        void     initialize();
        void     postInitialize();
        void     draw();
        uint32_t pick(glm::vec2 click_uv);
        void     recreateFramebuffer();

        VkDescriptorSetLayout                          _per_mesh_layout;
        MeshInefficientPickPerframeStorageBufferObject _mesh_inefficient_pick_perframe_storage_buffer_object;

    private:
        void setupAttachments();
        void setupRenderPass();
        void setupFramebuffer();
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();

    private:
        VkImage        _object_id_image        = VK_NULL_HANDLE;
        VkDeviceMemory _object_id_image_memory = VK_NULL_HANDLE;
        VkImageView    _object_id_image_view   = VK_NULL_HANDLE;
    };
} // namespace Pilot
