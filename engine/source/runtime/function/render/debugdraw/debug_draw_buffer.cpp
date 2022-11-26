#include "debug_draw_buffer.h"
#include <stdexcept>
#include "runtime/function/global/global_context.h"
#include "runtime/function/render/render_system.h"

namespace Piccolo
{
    void DebugDrawAllocator::initialize(DebugDrawFont* font)
    { 
        m_rhi = g_runtime_global_context.m_render_system->getRHI();
        m_font = font;
        setupDescriptorSet(); 
    }
    void DebugDrawAllocator::destory()
    {
        clear();
        unloadMeshBuffer();
    }

    void DebugDrawAllocator::tick()
    {
        flushPendingDelete();
        m_current_frame = (m_current_frame + 1) % k_deferred_delete_resource_frame_count;
    }

    RHIBuffer* DebugDrawAllocator::getVertexBuffer(){return m_vertex_resource.buffer;}
    RHIDescriptorSet* &DebugDrawAllocator::getDescriptorSet() { return m_descriptor.descriptor_set[m_rhi->getCurrentFrameIndex()]; }

    size_t DebugDrawAllocator::cacheVertexs(const std::vector<DebugDrawVertex>& vertexs)
    {
        size_t offset = m_vertex_cache.size();
        m_vertex_cache.resize(offset + vertexs.size());
        for (size_t i = 0; i < vertexs.size(); i++)
        {
            m_vertex_cache[i + offset] = vertexs[i];
        }
        return offset;
    }
    void DebugDrawAllocator::cacheUniformObject(Matrix4x4 proj_view_matrix)
    {
        m_uniform_buffer_object.proj_view_matrix = proj_view_matrix;
    }
    size_t DebugDrawAllocator::cacheUniformDynamicObject(const std::vector<std::pair<Matrix4x4, Vector4> >& model_colors)
    {
        size_t offset = m_uniform_buffer_dynamic_object_cache.size();
        m_uniform_buffer_dynamic_object_cache.resize(offset + model_colors.size());
        for (size_t i = 0; i < model_colors.size(); i++)
        {
            m_uniform_buffer_dynamic_object_cache[i + offset].model_matrix = model_colors[i].first;
            m_uniform_buffer_dynamic_object_cache[i + offset].color = model_colors[i].second;
        }
        return offset;
    }

    size_t DebugDrawAllocator::getVertexCacheOffset() const
    {
        return m_vertex_cache.size();
    }
    size_t DebugDrawAllocator::getUniformDynamicCacheOffset() const
    {
        return m_uniform_buffer_dynamic_object_cache.size();
    }

    void DebugDrawAllocator::allocator()
    {

        clearBuffer();
        uint64_t vertex_bufferSize = static_cast<uint64_t>(m_vertex_cache.size() * sizeof(DebugDrawVertex));
        if (vertex_bufferSize > 0)
        {
            m_rhi->createBuffer(
                vertex_bufferSize,
                RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_vertex_resource.buffer,
                m_vertex_resource.memory);

            void* data;
            m_rhi->mapMemory(m_vertex_resource.memory, 0, vertex_bufferSize, 0, &data);
            memcpy(data, m_vertex_cache.data(), vertex_bufferSize);
            m_rhi->unmapMemory(m_vertex_resource.memory);
        }

        uint64_t uniform_BufferSize = static_cast<uint64_t>(sizeof(UniformBufferObject));
        if (uniform_BufferSize > 0)
        {
            m_rhi->createBuffer(
                uniform_BufferSize,
                RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_uniform_resource.buffer,
                m_uniform_resource.memory);
            
            void* data;
            m_rhi->mapMemory(m_uniform_resource.memory, 0, uniform_BufferSize, 0, &data);
            memcpy(data, &m_uniform_buffer_object.proj_view_matrix, uniform_BufferSize);
            m_rhi->unmapMemory(m_uniform_resource.memory);
        }

        uint64_t uniform_dynamic_BufferSize = static_cast<uint64_t>(sizeof(UniformBufferDynamicObject) * m_uniform_buffer_dynamic_object_cache.size());
        if (uniform_dynamic_BufferSize > 0)
        {
            m_rhi->createBuffer(
                uniform_dynamic_BufferSize,
                RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_uniform_dynamic_resource.buffer,
                m_uniform_dynamic_resource.memory);
            
            void* data;
            m_rhi->mapMemory(m_uniform_dynamic_resource.memory, 0, uniform_dynamic_BufferSize, 0, &data);
            memcpy(data, m_uniform_buffer_dynamic_object_cache.data(), uniform_dynamic_BufferSize);
            m_rhi->unmapMemory(m_uniform_dynamic_resource.memory);
        }
        
        updateDescriptorSet();
    }

    void DebugDrawAllocator::clear()
    {
        clearBuffer();
        m_vertex_cache.clear();
        m_uniform_buffer_object.proj_view_matrix = Matrix4x4::IDENTITY;
        m_uniform_buffer_dynamic_object_cache.clear();
    }

    void DebugDrawAllocator::clearBuffer()
    {
        if (m_vertex_resource.buffer)
        {
            m_deffer_delete_queue[m_current_frame].push(m_vertex_resource);
            m_vertex_resource.buffer = nullptr;
            m_vertex_resource.memory = nullptr;
        }
        if (m_uniform_resource.buffer)
        {
            m_deffer_delete_queue[m_current_frame].push(m_uniform_resource);
            m_uniform_resource.buffer = nullptr;
            m_uniform_resource.memory = nullptr;
        }
        if (m_uniform_dynamic_resource.buffer)
        {
            m_deffer_delete_queue[m_current_frame].push(m_uniform_dynamic_resource);
            m_uniform_dynamic_resource.buffer = nullptr;
            m_uniform_dynamic_resource.memory = nullptr;
        }
    }

    void DebugDrawAllocator::flushPendingDelete()
    {
        uint32_t current_frame_to_delete = (m_current_frame + 1) % k_deferred_delete_resource_frame_count;
        while (!m_deffer_delete_queue[current_frame_to_delete].empty())
        {
            Resource resource_to_delete = m_deffer_delete_queue[current_frame_to_delete].front();
            m_deffer_delete_queue[current_frame_to_delete].pop();
            if (resource_to_delete.buffer == nullptr)continue;
            m_rhi->freeMemory(resource_to_delete.memory);
            m_rhi->destroyBuffer(resource_to_delete.buffer);
        }
    }

    void DebugDrawAllocator::setupDescriptorSet()
    {
        RHIDescriptorSetLayoutBinding uboLayoutBinding[3];
        uboLayoutBinding[0].binding = 0;
        uboLayoutBinding[0].descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding[0].descriptorCount = 1;
        uboLayoutBinding[0].stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding[0].pImmutableSamplers = nullptr;

        uboLayoutBinding[1].binding = 1;
        uboLayoutBinding[1].descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        uboLayoutBinding[1].descriptorCount = 1;
        uboLayoutBinding[1].stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding[1].pImmutableSamplers = nullptr;

        uboLayoutBinding[2].binding = 2;
        uboLayoutBinding[2].descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        uboLayoutBinding[2].descriptorCount = 1;
        uboLayoutBinding[2].stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding[2].pImmutableSamplers = nullptr;

        RHIDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 3;
        layoutInfo.pBindings = uboLayoutBinding;

        if (m_rhi->createDescriptorSetLayout(&layoutInfo, m_descriptor.layout) != RHI_SUCCESS)
        {
            throw std::runtime_error("create debug draw layout");
        }

        m_descriptor.descriptor_set.resize(m_rhi->getMaxFramesInFlight());
        for (size_t i = 0; i < m_rhi->getMaxFramesInFlight(); i++)
        {
            RHIDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.pNext = NULL;
            allocInfo.descriptorPool = m_rhi->getDescriptorPoor();
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &m_descriptor.layout;

            if (RHI_SUCCESS != m_rhi->allocateDescriptorSets(&allocInfo, m_descriptor.descriptor_set[i]))
            {
                throw std::runtime_error("debug draw descriptor set");
            }
        }

        prepareDescriptorSet();
    }

    //prepare at the start tick
    void DebugDrawAllocator::prepareDescriptorSet()
    {
        RHIDescriptorImageInfo image_info[1];
        image_info[0].imageView = m_font->getImageView();
        image_info[0].sampler = m_rhi->getOrCreateDefaultSampler(Default_Sampler_Linear);
        image_info[0].imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        for (size_t i = 0; i < m_rhi->getMaxFramesInFlight(); i++)
        {
            RHIWriteDescriptorSet descriptor_write[1];
            descriptor_write[0].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write[0].dstSet = m_descriptor.descriptor_set[i];
            descriptor_write[0].dstBinding = 2;
            descriptor_write[0].dstArrayElement = 0;
            descriptor_write[0].pNext = nullptr;
            descriptor_write[0].descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_write[0].descriptorCount = 1;
            descriptor_write[0].pBufferInfo = nullptr;
            descriptor_write[0].pImageInfo = &image_info[0];
            descriptor_write[0].pTexelBufferView = nullptr;

            m_rhi->updateDescriptorSets(1, descriptor_write, 0, nullptr);
        }
    }

    //update every tick
    void DebugDrawAllocator::updateDescriptorSet()
    {
        RHIDescriptorBufferInfo buffer_info[2];
        buffer_info[0].buffer = m_uniform_resource.buffer;
        buffer_info[0].offset = 0;
        buffer_info[0].range = sizeof(UniformBufferObject);

        buffer_info[1].buffer = m_uniform_dynamic_resource.buffer;
        buffer_info[1].offset = 0;
        buffer_info[1].range = sizeof(UniformBufferDynamicObject);
        
        RHIWriteDescriptorSet descriptor_write[2];
        descriptor_write[0].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write[0].dstSet = m_descriptor.descriptor_set[m_rhi->getCurrentFrameIndex()];
        descriptor_write[0].dstBinding = 0;
        descriptor_write[0].dstArrayElement = 0;
        descriptor_write[0].pNext = nullptr;
        descriptor_write[0].descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write[0].descriptorCount = 1;
        descriptor_write[0].pBufferInfo = &buffer_info[0];
        descriptor_write[0].pImageInfo = nullptr;
        descriptor_write[0].pTexelBufferView = nullptr;

        descriptor_write[1].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write[1].dstSet = m_descriptor.descriptor_set[m_rhi->getCurrentFrameIndex()];
        descriptor_write[1].dstBinding = 1;
        descriptor_write[1].dstArrayElement = 0;
        descriptor_write[1].pNext = nullptr;
        descriptor_write[1].descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptor_write[1].descriptorCount = 1;
        descriptor_write[1].pBufferInfo = &buffer_info[1];
        descriptor_write[1].pImageInfo = nullptr;
        descriptor_write[1].pTexelBufferView = nullptr;

        m_rhi->updateDescriptorSets(2, descriptor_write, 0, nullptr);
    }

    void DebugDrawAllocator::unloadMeshBuffer()
    {
        m_deffer_delete_queue[m_current_frame].push(m_sphere_resource);
        m_sphere_resource.buffer = nullptr;
        m_sphere_resource.memory = nullptr;
        m_deffer_delete_queue[m_current_frame].push(m_cylinder_resource);
        m_cylinder_resource.buffer = nullptr;
        m_cylinder_resource.memory = nullptr;
        m_deffer_delete_queue[m_current_frame].push(m_capsule_resource);
        m_capsule_resource.buffer = nullptr;
        m_capsule_resource.memory = nullptr;
    }
    
    void DebugDrawAllocator::loadSphereMeshBuffer()
    {
        int32_t param = m_circle_sample_count;
        //radios is 1
        float _2pi = 2.0f * Math_PI;
        std::vector<DebugDrawVertex> vertexs((param * 2 + 2) * (param * 2) * 2 + (param * 2 + 1) * (param * 2) * 2);

        int32_t current_index = 0;
        for (int32_t i = -param - 1; i < param + 1; i++)
        {
            float h = Math::sin(_2pi / 4.0f * i / (param + 1.0f));
            float h1 = Math::sin(_2pi / 4.0f * (i + 1) / (param + 1.0f));
            float r = Math::sqrt(1.0f - h * h);
            float r1 = Math::sqrt(1.0f - h1 * h1);
            for (int32_t j = 0; j < 2 * param; j++)
            {
                Vector3 p(Math::cos(_2pi / (2.0f * param) * j) * r, Math::sin(_2pi / (2.0f * param) * j) * r, h);
                Vector3 p1(Math::cos(_2pi / (2.0f * param) * j) * r1, Math::sin(_2pi / (2.0f * param) * j) * r1, h1);
                vertexs[current_index].pos = p;
                vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

                vertexs[current_index].pos = p1;
                vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            }
            if (i != -param - 1)
            {
                for (int32_t j = 0; j < 2 * param; j++)
                {
                    Vector3 p(Math::cos(_2pi / (2.0f * param) * j) * r, Math::sin(_2pi / (2.0f * param) * j) * r, h);
                    Vector3 p1(Math::cos(_2pi / (2.0f * param) * (j + 1)) * r, Math::sin(_2pi / (2.0f * param) * (j + 1)) * r, h);
                    vertexs[current_index].pos = p;
                    vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexs[current_index].pos = p1;
                    vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                }
            }
        }

        uint64_t bufferSize = static_cast<uint64_t>(vertexs.size() * sizeof(DebugDrawVertex));

        m_rhi->createBuffer(
            bufferSize,
            RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY,
            m_sphere_resource.buffer,
            m_sphere_resource.memory);

        Resource stagingBuffer;
        m_rhi->createBuffer(
            bufferSize,
            RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
            RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer.buffer,
            stagingBuffer.memory);
        void* data;
        m_rhi->mapMemory(stagingBuffer.memory, 0, bufferSize, 0, &data);
        memcpy(data, vertexs.data(), bufferSize);
        m_rhi->unmapMemory(stagingBuffer.memory);

        m_rhi->copyBuffer(stagingBuffer.buffer, m_sphere_resource.buffer, 0, 0, bufferSize);

        m_rhi->destroyBuffer(stagingBuffer.buffer);
        m_rhi->freeMemory(stagingBuffer.memory);
    }

    void DebugDrawAllocator::loadCylinderMeshBuffer()
    {
        int param = m_circle_sample_count;
        //radios is 1 , height is 2
        float _2pi = 2.0f * Math_PI;
        std::vector<DebugDrawVertex> vertexs(2 * param * 5 * 2);

        size_t current_index = 0;
        for (int32_t i = 0; i < 2 * param; i++)
        {
            Vector3 p(Math::cos(_2pi / (2.0f * param) * i), Math::sin(_2pi / (2.0f * param) * i), 1.0f);
            Vector3 p_(Math::cos(_2pi / (2.0f * param) * (i + 1)), Math::sin(_2pi / (2.0f * param) * (i + 1)), 1.0f);
            Vector3 p1(Math::cos(_2pi / (2.0f * param) * i), Math::sin(_2pi / (2.0f * param) * i), -1.0f);
            Vector3 p1_(Math::cos(_2pi / (2.0f * param) * (i + 1)), Math::sin(_2pi / (2.0f * param) * (i + 1)), -1.0f);

            vertexs[current_index].pos = p;
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            vertexs[current_index].pos = p_;
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

            vertexs[current_index].pos = p1;
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            vertexs[current_index].pos = p1_;
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
        
            vertexs[current_index].pos = p;
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            vertexs[current_index].pos = p1;
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
        
            vertexs[current_index].pos = p;
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            vertexs[current_index].pos = Vector3(0.0f, 0.0f, 1.0f);
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

            vertexs[current_index].pos = p1;
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            vertexs[current_index].pos = Vector3(0.0f, 0.0f, -1.0f);
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
        }

        uint64_t bufferSize = static_cast<uint64_t>(vertexs.size() * sizeof(DebugDrawVertex));

        m_rhi->createBuffer(
            bufferSize,
            RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY,
            m_cylinder_resource.buffer,
            m_cylinder_resource.memory);

        Resource stagingBuffer;
        m_rhi->createBuffer(
            bufferSize,
            RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
            RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer.buffer,
            stagingBuffer.memory);
        void* data;
        m_rhi->mapMemory(stagingBuffer.memory, 0, bufferSize, 0, &data);
        memcpy(data, vertexs.data(), bufferSize);
        m_rhi->unmapMemory(stagingBuffer.memory);

        m_rhi->copyBuffer(stagingBuffer.buffer, m_cylinder_resource.buffer, 0, 0, bufferSize);

        m_rhi->destroyBuffer(stagingBuffer.buffer);
        m_rhi->freeMemory(stagingBuffer.memory);
    }

    void DebugDrawAllocator::loadCapsuleMeshBuffer()
    {
        int param = m_circle_sample_count;
        //radios is 1,height is 4
        float _2pi = 2.0f * Math_PI;
        std::vector<DebugDrawVertex> vertexs(2 * param * param * 4 + 2 * param * param * 4 + 2 * param * 2);

        size_t current_index = 0;
        for (int32_t i = 0; i < param; i++)
        {
            float h = Math::sin(_2pi / 4.0 / param * i);
            float h1 = Math::sin(_2pi / 4.0 / param * (i + 1));
            float r = Math::sqrt(1 - h * h);
            float r1 = Math::sqrt(1 - h1 * h1);
            for (int32_t j = 0; j < 2 * param; j++)
            {
                Vector3 p(Math::cos(_2pi / (2.0f * param) * j) * r, Math::sin(_2pi / (2.0f * param) * j) * r, h + 1.0f);
                Vector3 p_(Math::cos(_2pi / (2.0f * param) * (j + 1)) * r, Math::sin(_2pi / (2.0f * param) * (j + 1)) * r, h + 1.0f);
                Vector3 p1(Math::cos(_2pi / (2.0f * param) * j) * r1, Math::sin(_2pi / (2.0f * param) * j) * r1, h1 + 1.0f);
                vertexs[current_index].pos = p;
                vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                vertexs[current_index].pos = p1;
                vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                
                vertexs[current_index].pos = p;
                vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                vertexs[current_index].pos = p_;
                vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            }
        }

        for (int32_t j = 0; j < 2 * param; j++)
        {
            Vector3 p(Math::cos(_2pi / (2.0f * param) * j), Math::sin(_2pi / (2.0f * param) * j), 1.0f);
            Vector3 p1(Math::cos(_2pi / (2.0f * param) * j), Math::sin(_2pi / (2.0f * param) * j), -1.0f);
            vertexs[current_index].pos = p;
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            vertexs[current_index].pos = p1;
            vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
        }

        for (int32_t i = 0; i > -param ; i--)
        {
            float h = Math::sin(_2pi / 4.0f / param * i);
            float h1 = Math::sin(_2pi / 4.0f / param * (i - 1));
            float r = Math::sqrt(1 - h * h);
            float r1 = Math::sqrt(1 - h1 * h1);
            for (int32_t j = 0; j < (2 * param); j++)
            {
                Vector3 p(Math::cos(_2pi / (2.0f * param) * j) * r, Math::sin(_2pi / (2.0f * param) * j) * r, h - 1.0f);
                Vector3 p_(Math::cos(_2pi / (2.0f * param) * (j + 1)) * r, Math::sin(_2pi / (2.0f * param) * (j + 1)) * r, h - 1.0f);
                Vector3 p1(Math::cos(_2pi / (2.0f * param) * j) * r1, Math::sin(_2pi / (2.0f * param) * j) * r1, h1 - 1.0f);
                vertexs[current_index].pos = p;
                vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                vertexs[current_index].pos = p1;
                vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

                vertexs[current_index].pos = p;
                vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                vertexs[current_index].pos = p_;
                vertexs[current_index++].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            }
        }

        uint64_t bufferSize = static_cast<uint64_t>(vertexs.size() * sizeof(DebugDrawVertex));

        m_rhi->createBuffer(
            bufferSize,
            RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY,
            m_capsule_resource.buffer,
            m_capsule_resource.memory);

        Resource stagingBuffer;
        m_rhi->createBuffer(
            bufferSize,
            RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
            RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer.buffer,
            stagingBuffer.memory);
        void* data;
        m_rhi->mapMemory(stagingBuffer.memory, 0, bufferSize, 0, &data);
        memcpy(data, vertexs.data(), bufferSize);
        m_rhi->unmapMemory(stagingBuffer.memory);

        m_rhi->copyBuffer(stagingBuffer.buffer, m_capsule_resource.buffer, 0, 0, bufferSize);

        m_rhi->destroyBuffer(stagingBuffer.buffer);
        m_rhi->freeMemory(stagingBuffer.memory);
    }

    
    RHIBuffer* DebugDrawAllocator::getSphereVertexBuffer()
    {
        if (m_sphere_resource.buffer == nullptr)
        {
            loadSphereMeshBuffer();
        }
        return m_sphere_resource.buffer;
    }
    RHIBuffer* DebugDrawAllocator::getCylinderVertexBuffer()
    {
        if (m_cylinder_resource.buffer == nullptr)
        {
            loadCylinderMeshBuffer();
        }
        return m_cylinder_resource.buffer;
    }
    RHIBuffer* DebugDrawAllocator::getCapsuleVertexBuffer()
    {
        if (m_capsule_resource.buffer == nullptr)
        {
            loadCapsuleMeshBuffer();
        }
        return m_capsule_resource.buffer;
    }

    const size_t DebugDrawAllocator::getSphereVertexBufferSize() const
    {
        return ((m_circle_sample_count * 2 + 2) * (m_circle_sample_count * 2) * 2 + (m_circle_sample_count * 2 + 1) * (m_circle_sample_count * 2) * 2);
    }
    const size_t DebugDrawAllocator::getCylinderVertexBufferSize() const
    {
        return (m_circle_sample_count * 2) * 5 * 2;
    }
    const size_t DebugDrawAllocator::getCapsuleVertexBufferSize() const
    {
        return (m_circle_sample_count * 2) * m_circle_sample_count * 4 + (2 * m_circle_sample_count) * 2 + (2 * m_circle_sample_count) * m_circle_sample_count * 4;
    }
    const size_t DebugDrawAllocator::getCapsuleVertexBufferUpSize() const
    {
        return (m_circle_sample_count * 2) * m_circle_sample_count * 4;
    }
    const size_t DebugDrawAllocator::getCapsuleVertexBufferMidSize() const
    {
        return 2 * m_circle_sample_count * 2;
    }
    const size_t DebugDrawAllocator::getCapsuleVertexBufferDownSize() const
    {
        return 2 * m_circle_sample_count * m_circle_sample_count * 4;
    }

    const size_t DebugDrawAllocator::getSizeOfUniformBufferObject() const
    {
        return sizeof(UniformBufferDynamicObject);
    }
}