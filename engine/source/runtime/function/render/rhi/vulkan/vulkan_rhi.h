#pragma once

#include "runtime/function/render/rhi.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <functional>
#include <optional>
#include <vector>

namespace Pilot
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        bool isComplete() { return graphics_family.has_value() && present_family.has_value(); }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    class VulkanRHI final : public RHI
    {
    public:
        // override functions
        virtual ~VulkanRHI() override final;
        virtual void initialize(RHIInitInfo init_info) override final;
        virtual void prepareContext() override final;

        // command
        VkCommandBuffer beginSingleTimeCommands();
        void            endSingleTimeCommands(VkCommandBuffer command_buffer);

        // swapchain
        void createSwapchain();
        void clearSwapchain();
        void recreateSwapchain();

        void createSwapchainImageViews();
        void createFramebufferImageAndView();

        // debug utilities label
        PFN_vkCmdBeginDebugUtilsLabelEXT _vkCmdBeginDebugUtilsLabelEXT;
        PFN_vkCmdEndDebugUtilsLabelEXT   _vkCmdEndDebugUtilsLabelEXT;

        // clear
        void clear();

        void waitForFences();
        void resetCommandPool();
        bool prepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain);
        void submitRendering(std::function<void()> passUpdateAfterRecreateSwapchain);

    public:
        GLFWwindow*        _window {nullptr};
        VkInstance         _instance {VK_NULL_HANDLE};
        VkSurfaceKHR       _surface {VK_NULL_HANDLE};
        VkPhysicalDevice   _physical_device {VK_NULL_HANDLE};
        QueueFamilyIndices _queue_indices;
        VkDevice           _device {VK_NULL_HANDLE};
        VkFormat           _depth_image_format {VK_FORMAT_UNDEFINED};
        VkQueue            _graphics_queue {VK_NULL_HANDLE};
        VkQueue            _present_queue {VK_NULL_HANDLE};
        VkCommandPool      _command_pool {VK_NULL_HANDLE};

        VkSwapchainKHR           _swapchain {VK_NULL_HANDLE};
        VkFormat                 _swapchain_image_format {VK_FORMAT_UNDEFINED};
        VkExtent2D               _swapchain_extent;
        std::vector<VkImage>     _swapchain_images;
        std::vector<VkImageView> _swapchain_imageviews;

        VkImage        _depth_image {VK_NULL_HANDLE};
        VkDeviceMemory _depth_image_memory {VK_NULL_HANDLE};
        VkImageView    _depth_image_view {VK_NULL_HANDLE};

        std::vector<VkFramebuffer> _swapchain_framebuffers;

        // asset allocator use VMA library
        VmaAllocator _assets_allocator;

        // function pointers
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
        VkDescriptorPool _descriptor_pool;

        // command pool and buffers
        static uint8_t const _max_frames_in_flight {3};
        uint8_t              _current_frame_index {0};
        VkCommandPool        _command_pools[_max_frames_in_flight];
        VkCommandBuffer      _command_buffers[_max_frames_in_flight];
        VkSemaphore          _image_available_for_render_semaphores[_max_frames_in_flight];
        VkSemaphore          _image_finished_for_presentation_semaphores[_max_frames_in_flight];
        VkFence              _is_frame_in_flight_fences[_max_frames_in_flight];

        // TODO: set
        VkCommandBuffer  _current_command_buffer;
        uint8_t*         _p_current_frame_index {nullptr};
        VkCommandPool*   _p_command_pools {nullptr};
        VkCommandBuffer* _p_command_buffers {nullptr};
        VkViewport       _viewport;
        VkRect2D         _scissor;

        uint32_t _current_swapchain_image_index;

    private:
        const std::vector<char const*> m_validation_layers {"VK_LAYER_KHRONOS_validation"};
        uint32_t                       m_vulkan_api_version {VK_API_VERSION_1_0};

        std::vector<char const*> m_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    private:
        void createInstance();
        void initializeDebugMessenger();
        void createWindowSurface();
        void initializePhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();
        void createCommandBuffers();
        void createDescriptorPool();
        void createSyncPrimitives();
        void createAssetAllocator();

    private:
        bool                     checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        void                     populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
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
} // namespace Pilot
