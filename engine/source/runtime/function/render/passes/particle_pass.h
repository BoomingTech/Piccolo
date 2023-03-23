#pragma once

#include "runtime/function/particle/particle_common.h"
#include "runtime/function/particle/particle_manager.h"
#include "runtime/function/render/render_pass.h"
#include "runtime/function/render/render_resource.h"

namespace Piccolo
{
    struct ParticlePassInitInfo : RenderPassInitInfo
    {
        std::shared_ptr<ParticleManager> m_particle_manager;
    };

    class ParticleEmitterBufferBatch
    {
    public:
        RHIBuffer* m_position_render_buffer = nullptr;
        RHIBuffer* m_position_device_buffer = nullptr;
        RHIBuffer* m_position_host_buffer = nullptr;
        RHIBuffer* m_counter_device_buffer = nullptr;
        RHIBuffer* m_counter_host_buffer = nullptr;
        RHIBuffer* m_indirect_dispatch_argument_buffer = nullptr;
        RHIBuffer* m_alive_list_buffer = nullptr;
        RHIBuffer* m_alive_list_next_buffer = nullptr;
        RHIBuffer* m_dead_list_buffer = nullptr;
        RHIBuffer* m_particle_component_res_buffer = nullptr;

        RHIDeviceMemory* m_counter_host_memory = nullptr;
        RHIDeviceMemory* m_position_host_memory = nullptr;
        RHIDeviceMemory* m_position_device_memory = nullptr;
        RHIDeviceMemory* m_counter_device_memory = nullptr;
        RHIDeviceMemory* m_indirect_dispatch_argument_memory = nullptr;
        RHIDeviceMemory* m_alive_list_memory = nullptr;
        RHIDeviceMemory* m_alive_list_next_memory = nullptr;
        RHIDeviceMemory* m_dead_list_memory = nullptr;
        RHIDeviceMemory* m_particle_component_res_memory = nullptr;
        RHIDeviceMemory* m_position_render_memory = nullptr;

        void* m_emitter_desc_mapped {nullptr};

        ParticleEmitterDesc m_emitter_desc;

        uint32_t m_num_particle {0};
        void     freeUpBatch(std::shared_ptr<RHI> rhi);
    };

    class ParticlePass : public RenderPass
    {
    public:
        void initialize(const RenderPassInitInfo* init_info) override final;

        void preparePassData(std::shared_ptr<RenderResourceBase> render_resource) override final;

        void draw() override final;

        void simulate();

        void copyNormalAndDepthImage();

        void setDepthAndNormalImage(RHIImage* depth_image, RHIImage* normal_image);

        void setupParticlePass();

        void setRenderCommandBufferHandle(RHICommandBuffer* command_buffer);

        void setRenderPassHandle(RHIRenderPass* render_pass);

        void updateAfterFramebufferRecreate();

        void setEmitterCount(int count);

        void createEmitter(int id, const ParticleEmitterDesc& desc);

        void initializeEmitters();

        void setTickIndices(const std::vector<ParticleEmitterID>& tick_indices);

        void setTransformIndices(const std::vector<ParticleEmitterTransformDesc>& transform_indices);

    private:
        void updateUniformBuffer();

        void updateEmitterTransform();

        void setupAttachments();

        void setupDescriptorSetLayout();

        void prepareUniformBuffer();

        void setupPipelines();

        void allocateDescriptorSet();

        void updateDescriptorSet();

        void setupParticleDescriptorSet();

        RHIPipeline* m_kickoff_pipeline = nullptr;
        RHIPipeline* m_emit_pipeline = nullptr;
        RHIPipeline* m_simulate_pipeline = nullptr;

        RHICommandBuffer* m_compute_command_buffer = nullptr;
        RHICommandBuffer* m_render_command_buffer = nullptr;
        RHICommandBuffer* m_copy_command_buffer = nullptr;

        RHIBuffer* m_scene_uniform_buffer = nullptr;
        RHIBuffer* m_compute_uniform_buffer = nullptr;
        RHIBuffer* m_particle_billboard_uniform_buffer = nullptr;

        RHIViewport m_viewport_params;

        RHIFence* m_fence = nullptr;

        RHIImage*        m_src_depth_image = nullptr;
        RHIImage*        m_dst_normal_image = nullptr;
        RHIImage*        m_src_normal_image = nullptr;
        RHIImage*        m_dst_depth_image = nullptr;
        RHIImageView*    m_src_depth_image_view = nullptr;
        RHIImageView*    m_src_normal_image_view = nullptr;
        RHIDeviceMemory* m_dst_normal_image_memory = nullptr;
        RHIDeviceMemory* m_dst_depth_image_memory = nullptr;

        /*
         * particle rendering
         */
        RHIImage*       m_particle_billboard_texture_image = nullptr;
        RHIImageView*   m_particle_billboard_texture_image_view = nullptr;
        VmaAllocation m_particle_billboard_texture_vma_allocation;

        RHIImage*       m_piccolo_logo_texture_image = nullptr;
        RHIImageView*   m_piccolo_logo_texture_image_view = nullptr;
        VmaAllocation m_piccolo_logo_texture_vma_allocation;

        RHIRenderPass* m_render_pass = nullptr;

        ParticleBillboardPerframeStorageBufferObject m_particlebillboard_perframe_storage_buffer_object;
        ParticleCollisionPerframeStorageBufferObject m_particle_collision_perframe_storage_buffer_object;

        void* m_particle_compute_buffer_mapped {nullptr};
        void* m_particle_billboard_uniform_buffer_mapped {nullptr};
        void* m_scene_uniform_buffer_mapped {nullptr};

        struct uvec4
        {
            uint32_t x;
            uint32_t y;
            uint32_t z;
            uint32_t w;
        };

        struct computeUniformBufferObject
        {
            int     emit_gap;
            int     xemit_count;
            float   max_life;
            float   time_step;
            Vector4 pack; // randomness 3 | frame index 1
            Vector3 gravity;
            float   padding;
            uvec4   viewport; // x, y, width, height
            Vector4 extent;   // width, height, near, far
        } m_ubo;

        struct Particle
        {
            Vector3 pos;
            float   life;
            Vector3 vel;
            float   size_x;
            Vector3 acc;
            float   size_y;
            Vector4 color;
        };

        // indirect dispath parameter offset
        static const uint32_t s_argument_offset_emit     = 0;
        static const uint32_t s_argument_offset_simulate = s_argument_offset_emit + sizeof(uvec4);
        struct IndirectArgumemt
        {
            uvec4 emit_argument;
            uvec4 simulate_argument;
            int   alive_flap_bit;
        };

        struct ParticleCounter
        {
            int dead_count;
            int alive_count;
            int alive_count_after_sim;
            int emit_count;
        };

        std::vector<ParticleEmitterBufferBatch> m_emitter_buffer_batches;
        std::shared_ptr<ParticleManager>        m_particle_manager;

        DefaultRNG m_random_engine;

        int m_emitter_count;

        static constexpr bool s_verbose_particle_alive_info {false};

        std::vector<ParticleEmitterID> m_emitter_tick_indices;

        std::vector<ParticleEmitterTransformDesc> m_emitter_transform_indices;
    };
} // namespace Piccolo
