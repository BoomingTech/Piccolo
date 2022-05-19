#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS 1
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#endif

#include "runtime/core/math/math_headers.h"

#include "vulkan_context.h"
#include "vulkan_global_resource.h"
#include "vulkan_mesh.h"
#include "vulkan_misc.h"
#include "vulkan_passes.h"
#include "vulkan_util.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <array>
#include <chrono>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Pilot
{
    class PVulkanManager
    {
    public:
        PVulkanManager();

        // clear module resource
        void clear();

        // render frame
        void renderFrame(class Scene&                scene,
                         class PilotRenderer*        pilot_renderer,
                         struct SceneReleaseHandles& release_handles,
                         void*                       ui_state);

        // legacy
        void renderFrameForward(class Scene&                scene,
                               class PilotRenderer*        pilot_renderer,
                               struct SceneReleaseHandles& release_handles,
                               void*                       ui_state);

        // initialize vulkan from io->window
        int initialize(GLFWwindow* window, class Scene& scene, class PilotRenderer* pilot_renderer);

        // initialize ui
        void initializeUI(void* surface_ui);

        // for editor use
        void     updateUIRenderSceneViewport(VkViewport render_scene_viewport);
        uint32_t getGuidOfPickedMesh(Vector2 picked_uv);
        size_t   updateCursorOnAxis(int     axis_mode,
                                    Vector2 cursor_uv,
                                    Vector2 game_engine_window_size,
                                    float   camera_fov,
                                    Vector3 camera_forward,
                                    Vector3 camera_up,
                                    Vector3 camera_right,
                                    Vector3 camera_position);

        // rendering config
        static bool m_enable_validation_Layers;
        static bool m_enable_debug_untils_label;
        static bool m_enable_point_light_shadow;

    private:
        // initialize render passes
        bool initializeRenderPass();
        // initialize command pool
        bool initializeCommandPool();
        // description pool for uniform buffer and image sampler
        bool initializeDescriptorPool();
        // semaphore : signal an image is ready for rendering / presentation
        bool createSyncPrimitives();
        // allocate command buffer: for drawing commands
        bool initializeCommandBuffers();
        // swapchain clear or recreate
        void clearSwapChain();
        // recreate swapchain
        bool recreateSwapChain();

        // per frame synchronization
        void cullingAndSyncScene(class Scene&                scene,
                                 class PilotRenderer*        pilot_renderer,
                                 struct SceneReleaseHandles& release_handles);

        // prepare context
        void prepareContext();

        // get visiable objects
        void
        culling(class Scene& scene, class PilotRenderer* pilot_renderer, struct SceneReleaseHandles& release_handles);

        // vulkan context include device creation, default command buffer, etc
        PVulkanContext m_vulkan_context;

        // global rendering resource, include IBL data, global storage buffer
        PGlobalRenderResource m_global_render_resource;
        // include lighting, shadow, post process, mouse picking pass
        PDirectionalLightShadowPass m_directional_light_shadow_pass;
        PPointLightShadowPass       m_point_light_shadow_pass;
        PMainCameraPass             m_main_camera_pass;
        PColorGradingPass           m_color_grading_pass;
        PToneMappingPass            m_tone_mapping_pass;
    	PPixelPass                  m_pixel_pass;
        PUIPass                     m_ui_pass;
        PCombineUIPass              m_combine_ui_pass;
        PPickPass                   m_mouse_pick_pass;

        static uint32_t const m_max_frames_in_flight = 3;
        uint32_t              m_current_frame_index  = 0;

        // global descriptor pool
        VkDescriptorPool m_descriptor_pool;

        bool   m_is_show_axis = true;
        size_t m_selected_axis;

        // storage buffer objects
        MeshPerframeStorageBufferObject              m_mesh_perframe_storage_buffer_object;
        AxisStorageBufferObject                      m_axis_storage_buffer_object;
        ParticleBillboardPerframeStorageBufferObject m_particlebillboard_perframe_storage_buffer_object;

        MeshPointLightShadowPerframeStorageBufferObject m_mesh_point_light_shadow_perframe_storage_buffer_object;
        MeshDirectionalLightShadowPerframeStorageBufferObject
                                                       m_mesh_directional_light_shadow_perframe_storage_buffer_object;
        MeshInefficientPickPerframeStorageBufferObject m_mesh_inefficient_pick_perframe_storage_buffer_object;

        static uint32_t m_max_vertex_blending_mesh_count;
        static uint32_t m_max_material_count;

        // viewport info
        VkViewport m_viewport = {0, 0, 1280, 720, 0, 1};
        VkRect2D   m_scissor  = {{0, 0}, {1280, 720}};

        VkCommandPool   m_command_pools[m_max_frames_in_flight];
        VkCommandBuffer m_command_buffers[m_max_frames_in_flight];
        VkSemaphore     m_image_available_for_render_semaphores[m_max_frames_in_flight];
        VkSemaphore     m_image_finished_for_presentation_semaphores[m_max_frames_in_flight];
        VkFence         m_is_frame_in_flight_fences[m_max_frames_in_flight];

        // load IBL
        void updateGlobalTexturesForIBL(PIBLResourceData& ibl_resource_data);

        // load color grading
        void updateGlobalTexturesForColorGrading(PColorGradingResourceData& color_grading_resource_data);

        void initializeCubeMap(VkImage&             image,
                               VkImageView&         image_view,
                               VmaAllocation&       image_allocation,
                               uint32_t             texture_image_width,
                               uint32_t             texture_image_height,
                               std::array<void*, 6> texture_image_pixels,
                               PILOT_PIXEL_FORMAT   texture_image_format,
                               uint32_t             miplevels);

        void generateTextureMipMaps(VkImage  image,
                                    VkFormat image_format,
                                    uint32_t texture_width,
                                    uint32_t texture_height,
                                    uint32_t layers,
                                    uint32_t miplevels);

        // cached mesh
        std::map<size_t, VulkanMesh> m_vulkan_meshes;
        // cached material
        std::map<size_t, VulkanPBRMaterial> m_vulkan_pbr_materials;
        // visible objects (updated per frame)
        std::vector<PVulkanMeshNode>              m_directional_light_visible_mesh_nodes;
        std::vector<PVulkanMeshNode>              m_point_lights_visible_mesh_nodes;
        std::vector<PVulkanMeshNode>              m_main_camera_visible_mesh_nodes;
        PVulkanAxisNode                           m_axis_node;
        std::vector<PVulkanParticleBillboardNode> m_main_camera_visible_particlebillboard_nodes;

        // data uploading functions
        VulkanMesh&
        syncMesh(struct RenderMesh const& mesh, bool has_skeleton_binding_handle, class PilotRenderer* pilot_renderer);
        void               updateMeshData(bool                                          has_skeleton_binding_handle,
                                          uint32_t                                      index_buffer_size,
                                          void*                                         index_buffer_data,
                                          uint32_t                                      vertex_buffer_size,
                                          struct Mesh_PosNormalTangentTex0Vertex const* vertex_buffer_data,
                                          uint32_t                                      joint_binding_buffer_size,
                                          struct Mesh_VertexBinding const*              joint_binding_buffer_data,
                                          VulkanMesh&                                   now_mesh);
        bool               updateVertexBuffer(bool                                          has_skeleton_binding_handle,
                                              uint32_t                                      vertex_buffer_size,
                                              struct Mesh_PosNormalTangentTex0Vertex const* vertex_buffer_data,
                                              uint32_t                                      joint_binding_buffer_size,
                                              struct Mesh_VertexBinding const*              joint_binding_buffer_data,
                                              uint32_t                                      index_buffer_size,
                                              uint16_t*                                     index_buffer_data,
                                              VulkanMesh&                                   now_mesh);
        bool               updateIndexBuffer(uint32_t index_buffer_size, void* index_buffer_data, VulkanMesh& now_mesh);
        VulkanPBRMaterial& syncMaterial(struct Material const& material, class PilotRenderer* pilot_renderer);
        void               updateTextureImageData(const PTextureDataToUpdate& texture_data);
        bool               initializeTextureImage(VkImage&           image,
                                                  VkImageView&       image_view,
                                                  VmaAllocation&     image_allocation,
                                                  uint32_t           texture_image_width,
                                                  uint32_t           texture_image_height,
                                                  void*              texture_image_pixels,
                                                  PILOT_PIXEL_FORMAT texture_image_format,
                                                  uint32_t           miplevels = 0);
    };

} // namespace Pilot
