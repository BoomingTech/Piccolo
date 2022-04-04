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

#include "runtime/function/render/include/render/framebuffer.h"
#include "runtime/function/render/include/render/surface.h"

#include <functional>

namespace Pilot
{
    template<typename R, typename S>
    struct PRegister
    {
        typedef std::function<const R(S s)> GetPtr;
        typedef std::function<const R(S s)> FuncPtr;
    };
    class Surface;
    class PilotRenderer
    {
        std::shared_ptr<Surface> m_surface;
        float                    m_last_tick_time = 0;

    public:
        PilotRenderer() : m_surface(std::make_shared<Surface>(this)) {}

        void initialize();
        void clear();
        bool tick();

        void setSurfaceUI(std::shared_ptr<SurfaceUI> pui) { m_surface->setSurfaceUI(pui); }

        std::shared_ptr<Surface> getPSurface() const { return m_surface; };

        bool RegisterGetPtr(PRegister<const FrameBuffer*, const PilotRenderer*>::GetPtr get)
        {
            f_get_framebuffer = get;
            return true;
        }
        bool RegisterGetPtr(PRegister<const SceneMemory*, SceneResourceHandle>::GetPtr get)
        {
            f_get_memory = get;
            return true;
        }
        bool RegisterGetPtr(PRegister<const SceneImage*, TextureHandle>::GetPtr get)
        {
            f_get_image = get;
            return true;
        }
        bool RegisterFuncPtr(PRegister<void, MeshHandle>::FuncPtr add)
        {
            f_add_release_mesh = add;
            return true;
        }
        bool RegisterFuncPtr(PRegister<void, PMaterialHandle>::FuncPtr add)
        {
            f_add_release_material = add;
            return true;
        }
        bool RegisterFuncPtr(PRegister<void, SkeletonBindingBufferHandle>::FuncPtr add)
        {
            f_add_release_skeleton_binding = add;
            return true;
        }

        void   updateWindow(float pos_x, float pos_y, float width, float height) const;
        size_t updateCursorOnAxis(int axis_mode, const Vector2& cursor_uv, const Vector2& window_size) const;
        size_t getGuidOfPickedMesh(const Vector2& picked_uv) const;

        PRegister<const FrameBuffer*, const PilotRenderer*>::GetPtr f_get_framebuffer;
        PRegister<const SceneMemory*, SceneResourceHandle>::GetPtr  f_get_memory;
        PRegister<const SceneImage*, TextureHandle>::GetPtr         f_get_image;
        PRegister<void, MeshHandle>::FuncPtr                        f_add_release_mesh;
        PRegister<void, PMaterialHandle>::FuncPtr                   f_add_release_material;
        PRegister<void, SkeletonBindingBufferHandle>::FuncPtr       f_add_release_skeleton_binding;
    };
} // namespace Pilot
