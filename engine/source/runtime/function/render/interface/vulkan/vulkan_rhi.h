#pragma once

#include "runtime/function/render/interface/rhi.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi_resource.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <functional>
#include <map>
#include <vector>

namespace Piccolo
{
    

    class VulkanRHI final : public RHI
    {
    public:
        // initialize
        virtual void initialize(RHIInitInfo init_info) override final;
        virtual void prepareContext() override final;

        // allocate and create
        bool allocateCommandBuffers(const RHICommandBufferAllocateInfo* pAllocateInfo, RHICommandBuffer* &pCommandBuffers) override;
        bool allocateDescriptorSets(const RHIDescriptorSetAllocateInfo* pAllocateInfo, RHIDescriptorSet* &pDescriptorSets) override;
        void createSwapchain() override;
        void recreateSwapchain() override;
        void createSwapchainImageViews() override;
        void createFramebufferImageAndView() override;
        RHISampler* getOrCreateDefaultSampler(RHIDefaultSamplerType type) override;
        RHISampler* getOrCreateMipmapSampler(uint32_t width, uint32_t height) override;
        RHIShader* createShaderModule(const std::vector<unsigned char>& shader_code) override;
        void createBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer* &buffer, RHIDeviceMemory* &buffer_memory) override;
        void createBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& buffer_memory, RHIDeviceSize size, void* data = nullptr, int datasize = 0) override;
        bool createBufferVMA(VmaAllocator allocator,
            const RHIBufferCreateInfo* pBufferCreateInfo,
            const VmaAllocationCreateInfo* pAllocationCreateInfo,
            RHIBuffer* &pBuffer,
            VmaAllocation* pAllocation,
            VmaAllocationInfo* pAllocationInfo) override;
        bool createBufferWithAlignmentVMA(
            VmaAllocator allocator,
            const RHIBufferCreateInfo* pBufferCreateInfo,
            const VmaAllocationCreateInfo* pAllocationCreateInfo,
            RHIDeviceSize minAlignment,
            RHIBuffer* &pBuffer,
            VmaAllocation* pAllocation,
            VmaAllocationInfo* pAllocationInfo) override;
        void copyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size) override;
        void createImage(uint32_t image_width, uint32_t image_height, RHIFormat format, RHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags,
            RHIImage* &image, RHIDeviceMemory* &memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels) override;
        void createImageView(RHIImage* image, RHIFormat format, RHIImageAspectFlags image_aspect_flags, RHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels,
            RHIImageView* &image_view) override;
        void createGlobalImage(RHIImage* &image, RHIImageView* &image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels, RHIFormat texture_image_format, uint32_t miplevels = 0) override;
        void createCubeMap(RHIImage* &image, RHIImageView* &image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels, RHIFormat texture_image_format, uint32_t miplevels) override;
        bool createCommandPool(const RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool* &pCommandPool) override;
        bool createDescriptorPool(const RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool* &pDescriptorPool) override;
        bool createDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout* &pSetLayout) override;
        bool createFence(const RHIFenceCreateInfo* pCreateInfo, RHIFence* &pFence) override;
        bool createFramebuffer(const RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer* &pFramebuffer) override;
        bool createGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline* &pPipelines) override;
        bool createComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines) override;
        bool createPipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout* &pPipelineLayout) override;
        bool createRenderPass(const RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass* &pRenderPass) override;
        bool createSampler(const RHISamplerCreateInfo* pCreateInfo, RHISampler* &pSampler) override;
        bool createSemaphore(const RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore* &pSemaphore) override;

        // command and command write
        bool waitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout) override;
        bool resetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences) override;
        bool resetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags) override;
        bool beginCommandBufferPFN(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) override;
        bool endCommandBufferPFN(RHICommandBuffer* commandBuffer) override;
        void cmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const RHIRenderPassBeginInfo* pRenderPassBegin, RHISubpassContents contents) override;
        void cmdNextSubpassPFN(RHICommandBuffer* commandBuffer, RHISubpassContents contents) override;
        void cmdEndRenderPassPFN(RHICommandBuffer* commandBuffer) override;
        void cmdBindPipelinePFN(RHICommandBuffer* commandBuffer, RHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline) override;
        void cmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const RHIViewport* pViewports) override;
        void cmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const RHIRect2D* pScissors) override;
        void cmdBindVertexBuffersPFN(
            RHICommandBuffer* commandBuffer,
            uint32_t firstBinding,
            uint32_t bindingCount,
            RHIBuffer* const* pBuffers,
            const RHIDeviceSize* pOffsets) override;
        void cmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, RHIIndexType indexType) override;
        void cmdBindDescriptorSetsPFN(
            RHICommandBuffer* commandBuffer,
            RHIPipelineBindPoint pipelineBindPoint,
            RHIPipelineLayout* layout,
            uint32_t firstSet,
            uint32_t descriptorSetCount,
            const RHIDescriptorSet* const* pDescriptorSets,
            uint32_t dynamicOffsetCount,
            const uint32_t* pDynamicOffsets) override;
        void cmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;
        void cmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const RHIClearAttachment* pAttachments, uint32_t rectCount, const RHIClearRect* pRects) override;

        bool beginCommandBuffer(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) override;
        void cmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const RHIBufferImageCopy* pRegions) override;
        void cmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageAspectFlagBits srcFlag, RHIImage* dstImage, RHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height) override;
        void cmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, RHIBufferCopy* pRegions) override;
        void cmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
        void cmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
        void cmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset) override;
        void cmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const RHIImageMemoryBarrier* pImageMemoryBarriers) override;
        bool endCommandBuffer(RHICommandBuffer* commandBuffer) override;
        void updateDescriptorSets(uint32_t descriptorWriteCount, const RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const RHICopyDescriptorSet* pDescriptorCopies) override;
        bool queueSubmit(RHIQueue* queue, uint32_t submitCount, const RHISubmitInfo* pSubmits, RHIFence* fence) override;
        bool queueWaitIdle(RHIQueue* queue) override;
        void resetCommandPool() override;
        void waitForFences() override;
        bool waitForFences(uint32_t fenceCount, const RHIFence* const* pFences, RHIBool32 waitAll, uint64_t timeout);

        // query
        void getPhysicalDeviceProperties(RHIPhysicalDeviceProperties* pProperties) override;
        RHICommandBuffer* getCurrentCommandBuffer() const override;
        RHICommandBuffer* const* getCommandBufferList() const override;
        RHICommandPool* getCommandPoor() const override;
        RHIDescriptorPool* getDescriptorPoor()const override;
        RHIFence* const* getFenceList() const override;
        QueueFamilyIndices getQueueFamilyIndices() const override;
        RHIQueue* getGraphicsQueue() const override;
        RHIQueue* getComputeQueue() const override;
        RHISwapChainDesc getSwapchainInfo() override;
        RHIDepthImageDesc getDepthImageInfo() const override;
        uint8_t getMaxFramesInFlight() const override;
        uint8_t getCurrentFrameIndex() const override;
        void setCurrentFrameIndex(uint8_t index) override;

        // command write
        RHICommandBuffer* beginSingleTimeCommands() override;
        void            endSingleTimeCommands(RHICommandBuffer* command_buffer) override;
        bool prepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain) override;
        void submitRendering(std::function<void()> passUpdateAfterRecreateSwapchain) override;
        void pushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color) override;
        void popEvent(RHICommandBuffer* commond_buffer) override;

        // destory
        virtual ~VulkanRHI() override final;
        void clear() override;
        void clearSwapchain() override;
        void destroyDefaultSampler(RHIDefaultSamplerType type) override;
        void destroyMipmappedSampler() override;
        void destroyShaderModule(RHIShader* shader) override;
        void destroySemaphore(RHISemaphore* semaphore) override;
        void destroySampler(RHISampler* sampler) override;
        void destroyInstance(RHIInstance* instance) override;
        void destroyImageView(RHIImageView* imageView) override;
        void destroyImage(RHIImage* image) override;
        void destroyFramebuffer(RHIFramebuffer* framebuffer) override;
        void destroyFence(RHIFence* fence) override;
        void destroyDevice() override;
        void destroyCommandPool(RHICommandPool* commandPool) override;
        void destroyBuffer(RHIBuffer* &buffer) override;
        void freeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers) override;

        // memory
        void freeMemory(RHIDeviceMemory* &memory) override;
        bool mapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData) override;
        void unmapMemory(RHIDeviceMemory* memory) override;
        void invalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) override;
        void flushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) override;
        
        //semaphores
        RHISemaphore* &getTextureCopySemaphore(uint32_t index) override;
    public:
        static uint8_t const k_max_frames_in_flight {3};

        
        RHIQueue* m_graphics_queue{ nullptr };
        RHIQueue* m_compute_queue{ nullptr };

        RHIFormat m_swapchain_image_format{ RHI_FORMAT_UNDEFINED };
        std::vector<RHIImageView*> m_swapchain_imageviews;
        RHIExtent2D m_swapchain_extent;
        RHIViewport m_viewport;
        RHIRect2D m_scissor;

        RHIFormat m_depth_image_format{ RHI_FORMAT_UNDEFINED };
        RHIImageView* m_depth_image_view = new VulkanImageView();

        RHIFence* m_rhi_is_frame_in_flight_fences[k_max_frames_in_flight];

        RHIDescriptorPool* m_descriptor_pool = new VulkanDescriptorPool();

        RHICommandPool* m_rhi_command_pool; 

        RHICommandBuffer* m_command_buffers[k_max_frames_in_flight];
        RHICommandBuffer* m_current_command_buffer = new VulkanCommandBuffer();

        QueueFamilyIndices m_queue_indices;

        GLFWwindow*        m_window {nullptr};
        VkInstance         m_instance {nullptr};
        VkSurfaceKHR       m_surface {nullptr};
        VkPhysicalDevice   m_physical_device {nullptr};
        VkDevice           m_device {nullptr};
        VkQueue            m_present_queue {nullptr};

        VkSwapchainKHR           m_swapchain {nullptr};
        std::vector<VkImage>     m_swapchain_images;

        RHIImage*        m_depth_image = new VulkanImage();
        VkDeviceMemory m_depth_image_memory {nullptr};

        std::vector<VkFramebuffer> m_swapchain_framebuffers;

        // asset allocator use VMA library
        VmaAllocator m_assets_allocator;

        // function pointers
        PFN_vkCmdBeginDebugUtilsLabelEXT _vkCmdBeginDebugUtilsLabelEXT;
        PFN_vkCmdEndDebugUtilsLabelEXT   _vkCmdEndDebugUtilsLabelEXT;
        PFN_vkWaitForFences         _vkWaitForFences;
        PFN_vkResetFences           _vkResetFences;
        PFN_vkResetCommandPool      _vkResetCommandPool;
        PFN_vkBeginCommandBuffer    _vkBeginCommandBuffer;
        PFN_vkEndCommandBuffer      _vkEndCommandBuffer;
        PFN_vkCmdBeginRenderPass    _vkCmdBeginRenderPass;
        PFN_vkCmdNextSubpass        _vkCmdNextSubpass;
        PFN_vkCmdEndRenderPass      _vkCmdEndRenderPass;
        PFN_vkCmdBindPipeline       _vkCmdBindPipeline;
        PFN_vkCmdSetViewport        _vkCmdSetViewport;
        PFN_vkCmdSetScissor         _vkCmdSetScissor;
        PFN_vkCmdBindVertexBuffers  _vkCmdBindVertexBuffers;
        PFN_vkCmdBindIndexBuffer    _vkCmdBindIndexBuffer;
        PFN_vkCmdBindDescriptorSets _vkCmdBindDescriptorSets;
        PFN_vkCmdDrawIndexed        _vkCmdDrawIndexed;
        PFN_vkCmdClearAttachments   _vkCmdClearAttachments;

        // global descriptor pool
        VkDescriptorPool m_vk_descriptor_pool;

        // command pool and buffers
        uint8_t              m_current_frame_index {0};
        VkCommandPool        m_command_pools[k_max_frames_in_flight];
        VkCommandBuffer      m_vk_command_buffers[k_max_frames_in_flight];
        VkSemaphore          m_image_available_for_render_semaphores[k_max_frames_in_flight];
        VkSemaphore          m_image_finished_for_presentation_semaphores[k_max_frames_in_flight];
        RHISemaphore*        m_image_available_for_texturescopy_semaphores[k_max_frames_in_flight];
        VkFence              m_is_frame_in_flight_fences[k_max_frames_in_flight];

        // TODO: set
        VkCommandBuffer   m_vk_current_command_buffer;

        uint32_t m_current_swapchain_image_index;

    private:
        const std::vector<char const*> m_validation_layers {"VK_LAYER_KHRONOS_validation"};
        uint32_t                       m_vulkan_api_version {VK_API_VERSION_1_0};

        std::vector<char const*> m_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        // default sampler cache
        RHISampler* m_linear_sampler = nullptr;
        RHISampler* m_nearest_sampler = nullptr;
        std::map<uint32_t, RHISampler*> m_mipmap_sampler_map;

    private:
        void createInstance();
        void initializeDebugMessenger();
        void createWindowSurface();
        void initializePhysicalDevice();
        void createLogicalDevice();
        void createCommandPool() override;;
        void createCommandBuffers();
        void createDescriptorPool();
        void createSyncPrimitives();
        void createAssetAllocator();

    public:
        bool isPointLightShadowEnabled() override;

    private:
        bool m_enable_validation_Layers{ true };
        bool m_enable_debug_utils_label{ true };
        bool m_enable_point_light_shadow{ true };

        // used in descriptor pool creation
        uint32_t m_max_vertex_blending_mesh_count{ 256 };
        uint32_t m_max_material_count{ 256 };

        bool                     checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        void                     populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        VkDebugUtilsMessengerEXT m_debug_messenger = nullptr;
        VkResult                 createDebugUtilsMessengerEXT(VkInstance                                instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks*              pAllocator,
                                                              VkDebugUtilsMessengerEXT*                 pDebugMessenger);
        void                     destroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                                               VkDebugUtilsMessengerEXT     debugMessenger,
                                                               const VkAllocationCallbacks* pAllocator);

        QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice physical_device);
        bool                    checkDeviceExtensionSupport(VkPhysicalDevice physical_device);
        bool                    isDeviceSuitable(VkPhysicalDevice physical_device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physical_device);

        VkFormat findDepthFormat();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags         features);

        VkSurfaceFormatKHR
        chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& available_surface_formats);
        VkPresentModeKHR
                   chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities);
    };
} // namespace Piccolo
