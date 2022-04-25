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
void SurfaceRHI::initialize(SurfaceIO* io, const FrameBuffer* framebuffer)
{
    m_vulkan_manager = new PVulkanManager();
    m_vulkan_manager->initialize(io->m_window, *framebuffer->m_scene, m_pilot_renderer);
    m_window = io->m_window;
}

void SurfaceRHI::initializeUI(SurfaceUI* ui) { m_vulkan_manager->initializeUI(ui); }

void SurfaceRHI::tick(const FrameBuffer* framebuffer, SceneReleaseHandles& release_handles, UIState* ui_state)
{
    m_vulkan_manager->renderFrame(*framebuffer->m_scene, m_pilot_renderer, release_handles, ui_state);

    // legacy
    // m_vulkan_manager->renderFrameForward(*framebuffer->m_scene, m_pilot_renderer, release_handles, ui_state);
}

void SurfaceRHI::clear()
{
    m_vulkan_manager->clear();
    delete m_vulkan_manager;
}