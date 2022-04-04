#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#include "runtime/function/render/include/render/render.h"
#include "runtime/function/render/include/render/surface.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

using namespace Pilot;

// PSurfaceRHI : now vulkan rhi only
int SurfaceRHI::initialize(SurfaceIO* io, const FrameBuffer* framebuffer)
{
    m_vulkan_manager = new PVulkanManager(io->m_window, *framebuffer->m_scene, m_pilot_renderer);
    m_window         = io->m_window;
    return 0;
}

void SurfaceRHI::tick_pre(const FrameBuffer* framebuffer, SceneReleaseHandles& release_handles)
{
    // prepare material and mesh assets
    m_vulkan_manager->syncScene(*framebuffer->m_scene, m_pilot_renderer, release_handles);

    m_vulkan_manager->beginFrame();
}

void SurfaceRHI::tick_post(const FrameBuffer* framebuffer) { m_vulkan_manager->endFrame(); }
int  SurfaceRHI::clear()
{
    m_vulkan_manager->clear();
    delete m_vulkan_manager;

    return 0;
}