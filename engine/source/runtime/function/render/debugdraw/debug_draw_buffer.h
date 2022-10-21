#pragma once

#include "runtime/function/render/interface/rhi.h"
#include "debug_draw_primitive.h"
#include "debug_draw_font.h"

#include <queue>
namespace Piccolo
{
    class DebugDrawAllocator
    {
    public:
        DebugDrawAllocator() {};
        void initialize(DebugDrawFont* font);
        void destory();
        void tick();
        void clear();
        void clearBuffer();
        
        size_t cacheVertexs(const std::vector<DebugDrawVertex>& vertexs);
        void cacheUniformObject(Matrix4x4 proj_view_matrix);
        size_t cacheUniformDynamicObject(const std::vector<std::pair<Matrix4x4,Vector4> >& model_colors);

        size_t getVertexCacheOffset() const;
        size_t getUniformDynamicCacheOffset() const;
        void allocator();

        RHIBuffer* getVertexBuffer();
        RHIDescriptorSet* &getDescriptorSet();

        RHIBuffer* getSphereVertexBuffer();
        RHIBuffer* getCylinderVertexBuffer();
        RHIBuffer* getCapsuleVertexBuffer();

        const size_t getSphereVertexBufferSize() const;
        const size_t getCylinderVertexBufferSize() const;
        const size_t getCapsuleVertexBufferSize() const;
        const size_t getCapsuleVertexBufferUpSize() const;
        const size_t getCapsuleVertexBufferMidSize() const;
        const size_t getCapsuleVertexBufferDownSize() const;

        const size_t getSizeOfUniformBufferObject() const;
    private:
        std::shared_ptr<RHI> m_rhi;
        struct UniformBufferObject
        {
            Matrix4x4 proj_view_matrix;
        };

        struct alignas(256) UniformBufferDynamicObject
        {
            Matrix4x4 model_matrix;
            Vector4 color;
        };

        struct Resource
        {
            RHIBuffer* buffer = nullptr;
            RHIDeviceMemory* memory = nullptr;
        };
        struct Descriptor
        {
            RHIDescriptorSetLayout* layout = nullptr;
            std::vector<RHIDescriptorSet*> descriptor_set;
        };

        //descriptor
        Descriptor m_descriptor;

        //changeable resource
        Resource m_vertex_resource;
        std::vector<DebugDrawVertex>m_vertex_cache;

        Resource m_uniform_resource;
        UniformBufferObject m_uniform_buffer_object;

        Resource m_uniform_dynamic_resource;
        std::vector<UniformBufferDynamicObject> m_uniform_buffer_dynamic_object_cache;

        //static mesh resource
        Resource m_sphere_resource;
        Resource m_cylinder_resource;
        Resource m_capsule_resource;

        //font resource
        DebugDrawFont* m_font = nullptr;

        //resource deleter
        static const uint32_t k_deferred_delete_resource_frame_count = 5;//the count means after count-1 frame will be delete
        uint32_t m_current_frame = 0;
        std::queue<Resource> m_deffer_delete_queue[k_deferred_delete_resource_frame_count];

    private:
        void setupDescriptorSet();
        void prepareDescriptorSet();
        void updateDescriptorSet();
        void flushPendingDelete();
        void unloadMeshBuffer();
        void loadSphereMeshBuffer();
        void loadCylinderMeshBuffer();
        void loadCapsuleMeshBuffer();

        const int m_circle_sample_count = 10;
    };
}