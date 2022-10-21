#include "debug_draw_manager.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/render/render_system.h"
#include "runtime/core/math/math_headers.h"

namespace Piccolo
{
    void DebugDrawManager::initialize()
    {
        m_rhi = g_runtime_global_context.m_render_system->getRHI();
        setupPipelines();
    }

    void DebugDrawManager::setupPipelines()
    {
        //setup pipelines
        for (uint8_t i = 0; i < DebugDrawPipelineType::_debug_draw_pipeline_type_count; i++)
        {
            m_debug_draw_pipeline[i] = new DebugDrawPipeline((DebugDrawPipelineType)i);
            m_debug_draw_pipeline[i]->initialize();
        }
        m_buffer_allocator = new DebugDrawAllocator();
        m_font = new DebugDrawFont();
        m_font->inialize();
        m_buffer_allocator->initialize(m_font);
    }

    void DebugDrawManager::destory()
    {
        for (uint8_t i = 0; i < DebugDrawPipelineType::_debug_draw_pipeline_type_count; i++)
        {
            m_debug_draw_pipeline[i]->destory();
            delete m_debug_draw_pipeline[i];
        }

        m_buffer_allocator->destory();
        delete m_buffer_allocator;
        
        //m_font->destroy();
        delete m_font;
    }
    void DebugDrawManager::clear()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_debug_draw_context.clear();
    }

    void DebugDrawManager::tick(float delta_time)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_buffer_allocator->tick();
        m_debug_draw_context.tick(delta_time);
    }
    
    void DebugDrawManager::updateAfterRecreateSwapchain()
    {
        for (uint8_t i = 0; i < DebugDrawPipelineType::_debug_draw_pipeline_type_count; i++)
        {
            m_debug_draw_pipeline[i]->recreateAfterSwapchain();
        }
    }

    void DebugDrawManager::preparePassData(std::shared_ptr<RenderResourceBase> render_resource)
    {
        const RenderResource* resource = static_cast<const RenderResource*>(render_resource.get());
        m_proj_view_matrix = resource->m_mesh_perframe_storage_buffer_object.proj_view_matrix;

    }

    void DebugDrawManager::swapDataToRender()
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        m_debug_draw_group_for_render.clear();
        size_t debug_draw_group_count = m_debug_draw_context.m_debug_draw_groups.size();
        for (size_t debug_draw_group_index = 0; debug_draw_group_index < debug_draw_group_count; debug_draw_group_index++)
        {
            DebugDrawGroup* debug_draw_group = m_debug_draw_context.m_debug_draw_groups[debug_draw_group_index];
            if (debug_draw_group == nullptr)continue;
            m_debug_draw_group_for_render.mergeFrom(debug_draw_group);
        }
    }

    void DebugDrawManager::draw(uint32_t current_swapchain_image_index)
    {

        static uint32_t once = 1;
        swapDataToRender();
        once = 0;

        float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_rhi->pushEvent(m_rhi->getCurrentCommandBuffer(), "DebugDrawManager", color);
        m_rhi->cmdSetViewportPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, m_rhi->getSwapchainInfo().viewport);
        m_rhi->cmdSetScissorPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, m_rhi->getSwapchainInfo().scissor);

        drawDebugObject(current_swapchain_image_index);

        m_rhi->popEvent(m_rhi->getCurrentCommandBuffer());
    }

    void DebugDrawManager::prepareDrawBuffer()
    {
        m_buffer_allocator->clear();

        std::vector<DebugDrawVertex> vertexs;

        m_debug_draw_group_for_render.writePointData(vertexs, false);
        m_point_start_offset = m_buffer_allocator->cacheVertexs(vertexs);
        m_point_end_offset = m_buffer_allocator->getVertexCacheOffset();

        m_debug_draw_group_for_render.writeLineData(vertexs, false);
        m_line_start_offset = m_buffer_allocator->cacheVertexs(vertexs);
        m_line_end_offset = m_buffer_allocator->getVertexCacheOffset();

        m_debug_draw_group_for_render.writeTriangleData(vertexs, false);
        m_triangle_start_offset = m_buffer_allocator->cacheVertexs(vertexs);
        m_triangle_end_offset = m_buffer_allocator->getVertexCacheOffset();

        m_debug_draw_group_for_render.writePointData(vertexs, true);
        m_no_depth_test_point_start_offset = m_buffer_allocator->cacheVertexs(vertexs);
        m_no_depth_test_point_end_offset = m_buffer_allocator->getVertexCacheOffset();

        m_debug_draw_group_for_render.writeLineData(vertexs, true);
        m_no_depth_test_line_start_offset = m_buffer_allocator->cacheVertexs(vertexs);
        m_no_depth_test_line_end_offset = m_buffer_allocator->getVertexCacheOffset();

        m_debug_draw_group_for_render.writeTriangleData(vertexs, true);
        m_no_depth_test_triangle_start_offset = m_buffer_allocator->cacheVertexs(vertexs);
        m_no_depth_test_triangle_end_offset = m_buffer_allocator->getVertexCacheOffset();

        m_debug_draw_group_for_render.writeTextData(vertexs, m_font, m_proj_view_matrix);
        m_text_start_offset = m_buffer_allocator->cacheVertexs(vertexs);
        m_text_end_offset = m_buffer_allocator->getVertexCacheOffset();

        m_buffer_allocator->cacheUniformObject(m_proj_view_matrix);

        std::vector<std::pair<Matrix4x4, Vector4> > dynamicObject = { std::make_pair(Matrix4x4::IDENTITY,Vector4(0,0,0,0)) };
        m_buffer_allocator->cacheUniformDynamicObject(dynamicObject);//cache the first model matrix as Identity matrix, color as empty color. (default object)

        m_debug_draw_group_for_render.writeUniformDynamicDataToCache(dynamicObject);
        m_buffer_allocator->cacheUniformDynamicObject(dynamicObject);//cache the wire frame uniform dynamic object

        m_buffer_allocator->allocator();
    }

    void DebugDrawManager::drawPointLineTriangleBox(uint32_t current_swapchain_image_index)
    {
        // draw point, line ,triangle , triangle_without_depth_test
        RHIBuffer* vertex_buffers[] = { m_buffer_allocator->getVertexBuffer() };
        if (vertex_buffers[0] == nullptr)
        {
            return;
        }
        RHIDeviceSize offsets[] = { 0 };
        m_rhi->cmdBindVertexBuffersPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, vertex_buffers, offsets);

        std::vector<DebugDrawPipeline*>vc_pipelines{ m_debug_draw_pipeline[DebugDrawPipelineType::_debug_draw_pipeline_type_point],
                                                     m_debug_draw_pipeline[DebugDrawPipelineType::_debug_draw_pipeline_type_line],
                                                     m_debug_draw_pipeline[DebugDrawPipelineType::_debug_draw_pipeline_type_triangle],
                                                     m_debug_draw_pipeline[DebugDrawPipelineType::_debug_draw_pipeline_type_point_no_depth_test],
                                                     m_debug_draw_pipeline[DebugDrawPipelineType::_debug_draw_pipeline_type_line_no_depth_test],
                                                     m_debug_draw_pipeline[DebugDrawPipelineType::_debug_draw_pipeline_type_triangle_no_depth_test],
                                                     m_debug_draw_pipeline[DebugDrawPipelineType::_debug_draw_pipeline_type_triangle_no_depth_test] };
        std::vector<size_t>vc_start_offsets{ m_point_start_offset,
                                             m_line_start_offset,
                                             m_triangle_start_offset,
                                             m_no_depth_test_point_start_offset,
                                             m_no_depth_test_line_start_offset,
                                             m_no_depth_test_triangle_start_offset,
                                             m_text_start_offset };
        std::vector<size_t>vc_end_offsets{ m_point_end_offset,
                                           m_line_end_offset,
                                           m_triangle_end_offset,
                                           m_no_depth_test_point_end_offset,
                                           m_no_depth_test_line_end_offset,
                                           m_no_depth_test_triangle_end_offset,
                                           m_text_end_offset };
        RHIClearValue clear_values[2];
        clear_values[0].color = { 0.0f,0.0f,0.0f,0.0f };
        clear_values[1].depthStencil = { 1.0f, 0 };
        RHIRenderPassBeginInfo renderpass_begin_info{};
        renderpass_begin_info.sType = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_begin_info.renderArea.offset = { 0, 0 };
        renderpass_begin_info.renderArea.extent = m_rhi->getSwapchainInfo().extent;
        renderpass_begin_info.clearValueCount = (sizeof(clear_values) / sizeof(clear_values[0]));
        renderpass_begin_info.pClearValues = clear_values;

        for (size_t i = 0; i < vc_pipelines.size(); i++)
        {
            if (vc_end_offsets[i] - vc_start_offsets[i] == 0)
            {
                continue;
            }
            renderpass_begin_info.renderPass = vc_pipelines[i]->getFramebuffer().render_pass;
            renderpass_begin_info.framebuffer = vc_pipelines[i]->getFramebuffer().framebuffers[current_swapchain_image_index];
            m_rhi->cmdBeginRenderPassPFN(m_rhi->getCurrentCommandBuffer(), &renderpass_begin_info, RHI_SUBPASS_CONTENTS_INLINE);

            m_rhi->cmdBindPipelinePFN(m_rhi->getCurrentCommandBuffer(), RHI_PIPELINE_BIND_POINT_GRAPHICS, vc_pipelines[i]->getPipeline().pipeline);

            uint32_t dynamicOffset = 0;
            m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                RHI_PIPELINE_BIND_POINT_GRAPHICS,
                vc_pipelines[i]->getPipeline().layout,
                0,
                1,
                &m_buffer_allocator->getDescriptorSet(),
                1,
                &dynamicOffset);
            m_rhi->cmdDraw(m_rhi->getCurrentCommandBuffer(), vc_end_offsets[i] - vc_start_offsets[i], 1, vc_start_offsets[i], 0);

            m_rhi->cmdEndRenderPassPFN(m_rhi->getCurrentCommandBuffer());
        }
    }
    void DebugDrawManager::drawWireFrameObject(uint32_t current_swapchain_image_index)
    {
        //draw wire frame object : sphere, cylinder, capsule
        
        std::vector<DebugDrawPipeline*>vc_pipelines{ m_debug_draw_pipeline[DebugDrawPipelineType::_debug_draw_pipeline_type_line],
                                                     m_debug_draw_pipeline[DebugDrawPipelineType::_debug_draw_pipeline_type_line_no_depth_test] };
        std::vector<bool>no_depth_tests = { false,true };

        for (int32_t i = 0; i < 2; i++)
        {
            bool no_depth_test = no_depth_tests[i];

            RHIDeviceSize offsets[] = { 0 };
            RHIClearValue clear_values[2];
            clear_values[0].color = { 0.0f,0.0f,0.0f,0.0f };
            clear_values[1].depthStencil = { 1.0f, 0 };
            RHIRenderPassBeginInfo renderpass_begin_info{};
            renderpass_begin_info.sType = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpass_begin_info.renderArea.offset = { 0, 0 };
            renderpass_begin_info.renderArea.extent = m_rhi->getSwapchainInfo().extent;
            renderpass_begin_info.clearValueCount = (sizeof(clear_values) / sizeof(clear_values[0]));
            renderpass_begin_info.pClearValues = clear_values;
            renderpass_begin_info.renderPass = vc_pipelines[i]->getFramebuffer().render_pass;
            renderpass_begin_info.framebuffer = vc_pipelines[i]->getFramebuffer().framebuffers[current_swapchain_image_index];
            m_rhi->cmdBeginRenderPassPFN(m_rhi->getCurrentCommandBuffer(), &renderpass_begin_info, RHI_SUBPASS_CONTENTS_INLINE);
            m_rhi->cmdBindPipelinePFN(m_rhi->getCurrentCommandBuffer(), RHI_PIPELINE_BIND_POINT_GRAPHICS, vc_pipelines[i]->getPipeline().pipeline);

            size_t uniform_dynamic_size = m_buffer_allocator->getSizeOfUniformBufferObject();
            uint32_t dynamicOffset = uniform_dynamic_size;

            size_t sphere_count = m_debug_draw_group_for_render.getSphereCount(no_depth_test);
            size_t cylinder_count = m_debug_draw_group_for_render.getCylinderCount(no_depth_test);
            size_t capsule_count = m_debug_draw_group_for_render.getCapsuleCount(no_depth_test);

            if (sphere_count > 0)
            {
                RHIBuffer* sphere_vertex_buffers[] = { m_buffer_allocator->getSphereVertexBuffer() };
                m_rhi->cmdBindVertexBuffersPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, sphere_vertex_buffers, offsets);
                for (size_t j = 0; j < sphere_count; j++)
                {
                    
                    m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                        RHI_PIPELINE_BIND_POINT_GRAPHICS,
                        vc_pipelines[i]->getPipeline().layout,
                        0,
                        1,
                        &m_buffer_allocator->getDescriptorSet(),
                        1,
                        &dynamicOffset);
                    m_rhi->cmdDraw(m_rhi->getCurrentCommandBuffer(), m_buffer_allocator->getSphereVertexBufferSize(), 1, 0, 0);
                    dynamicOffset += uniform_dynamic_size;
                }
            }

            if (cylinder_count > 0)
            {
                RHIBuffer* cylinder_vertex_buffers[] = { m_buffer_allocator->getCylinderVertexBuffer() };
                m_rhi->cmdBindVertexBuffersPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, cylinder_vertex_buffers, offsets);
                for (size_t j = 0; j < cylinder_count; j++)
                {
                    m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                        RHI_PIPELINE_BIND_POINT_GRAPHICS,
                        vc_pipelines[i]->getPipeline().layout,
                        0,
                        1,
                        &m_buffer_allocator->getDescriptorSet(),
                        1,
                        &dynamicOffset);
                    m_rhi->cmdDraw(m_rhi->getCurrentCommandBuffer(), m_buffer_allocator->getCylinderVertexBufferSize(), 1, 0, 0);
                    dynamicOffset += uniform_dynamic_size;
                }
            }

            if (capsule_count > 0)
            {
                RHIBuffer* capsule_vertex_buffers[] = { m_buffer_allocator->getCapsuleVertexBuffer() };
                m_rhi->cmdBindVertexBuffersPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, capsule_vertex_buffers, offsets);
                for (size_t j = 0; j < capsule_count; j++)
                {
                    //draw capsule up part
                    m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                        RHI_PIPELINE_BIND_POINT_GRAPHICS,
                        vc_pipelines[i]->getPipeline().layout,
                        0,
                        1,
                        &m_buffer_allocator->getDescriptorSet(),
                        1,
                        &dynamicOffset);
                    m_rhi->cmdDraw(m_rhi->getCurrentCommandBuffer(), m_buffer_allocator->getCapsuleVertexBufferUpSize(), 1, 0, 0);
                    dynamicOffset += uniform_dynamic_size;

                    //draw capsule mid part
                    m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                        RHI_PIPELINE_BIND_POINT_GRAPHICS,
                        vc_pipelines[i]->getPipeline().layout,
                        0,
                        1,
                        &m_buffer_allocator->getDescriptorSet(),
                        1,
                        &dynamicOffset);
                    m_rhi->cmdDraw(m_rhi->getCurrentCommandBuffer(), m_buffer_allocator->getCapsuleVertexBufferMidSize(), 1, m_buffer_allocator->getCapsuleVertexBufferUpSize(), 0);
                    dynamicOffset += uniform_dynamic_size;

                    //draw capsule down part
                    m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                        RHI_PIPELINE_BIND_POINT_GRAPHICS,
                        vc_pipelines[i]->getPipeline().layout,
                        0,
                        1,
                        &m_buffer_allocator->getDescriptorSet(),
                        1,
                        &dynamicOffset);
                    m_rhi->cmdDraw(m_rhi->getCurrentCommandBuffer(),
                        m_buffer_allocator->getCapsuleVertexBufferDownSize(),
                        1,
                        m_buffer_allocator->getCapsuleVertexBufferUpSize() + m_buffer_allocator->getCapsuleVertexBufferMidSize(),
                        0);
                    dynamicOffset += uniform_dynamic_size;
                }
            }

            m_rhi->cmdEndRenderPassPFN(m_rhi->getCurrentCommandBuffer());
        }
    }

    void DebugDrawManager::drawDebugObject(uint32_t current_swapchain_image_index)
    {
        prepareDrawBuffer();
        drawPointLineTriangleBox(current_swapchain_image_index);
        drawWireFrameObject(current_swapchain_image_index);
    }

    DebugDrawGroup* DebugDrawManager::tryGetOrCreateDebugDrawGroup(const std::string& name)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        return m_debug_draw_context.tryGetOrCreateDebugDrawGroup(name);
    }
}
