#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

#include <vk_mem_alloc.h>

namespace Pilot
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    class PVulkanContext
    {
    public:
        GLFWwindow*        _window          = nullptr;
        VkInstance         _instance        = VK_NULL_HANDLE;
        VkSurfaceKHR       _surface         = VK_NULL_HANDLE;
        VkPhysicalDevice   _physical_device = VK_NULL_HANDLE;
        QueueFamilyIndices _queue_indices;
        VkDevice           _device             = VK_NULL_HANDLE;
        VkFormat           _depth_image_format = VK_FORMAT_UNDEFINED;
        VkQueue            _graphics_queue     = VK_NULL_HANDLE;
        VkQueue            _present_queue      = VK_NULL_HANDLE;
        VkCommandPool      _command_pool       = VK_NULL_HANDLE;

        VkSwapchainKHR           _swapchain              = VK_NULL_HANDLE;
        VkFormat                 _swapchain_image_format = VK_FORMAT_UNDEFINED;
        VkExtent2D               _swapchain_extent;
        std::vector<VkImage>     _swapchain_images;
        std::vector<VkImageView> _swapchain_imageviews;

        VkImage        _depth_image        = VK_NULL_HANDLE;
        VkDeviceMemory _depth_image_memory = VK_NULL_HANDLE;
        VkImageView    _depth_image_view   = VK_NULL_HANDLE;

        std::vector<VkFramebuffer> _swapchain_framebuffers;

        // asset allocator use VMA library
        VmaAllocator _assets_allocator;

        void initialize(GLFWwindow* window);
        void clear();

        VkCommandBuffer beginSingleTimeCommands();
        void            endSingleTimeCommands(VkCommandBuffer command_buffer);

        // debug functions
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

        void createSwapchain();
        void clearSwapchain();
        void recreateSwapchain();

        void createSwapchainImageViews();
        void createFramebufferImageAndView();

    private:
        const std::vector<char const*> m_validation_layers  = {"VK_LAYER_KHRONOS_validation"};
        uint32_t                       m_vulkan_api_version = VK_API_VERSION_1_0;

        std::vector<char const*> m_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    private:
        void createInstance();
        void initializeDebugMessenger();
        void createWindowSurface();
        void initializePhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();
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
