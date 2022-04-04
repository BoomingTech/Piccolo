#pragma once

#include "runtime/function/render/include/render/framebuffer.h"
#include "runtime/function/render/include/render/surface_io.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

#include <functional>

namespace Pilot
{
    class PilotRenderer;
    class PVulkanManager;
    class SurfaceRHI
    {
        PilotRenderer* m_pilot_renderer;
        PRenderPath    m_curpath {PRenderPath::Clustered};

    public:
        GLFWwindow*     m_window;
        PVulkanManager* m_vulkan_manager;

        SurfaceRHI(PilotRenderer* prenderer) : m_pilot_renderer(prenderer) {}
        ~SurfaceRHI() { delete m_vulkan_manager; }
        int  initialize(SurfaceIO* io, const FrameBuffer* framebuffer);
        void tick_pre(const FrameBuffer* framebuffer, SceneReleaseHandles& release_handles);
        void tick_post(const FrameBuffer* framebuffer);
        int  clear();
    };
    class SurfaceUI
    {
    public:
        UIState*                   m_tmp_uistate {nullptr};
        SurfaceRHI*                m_rhi;
        std::shared_ptr<SurfaceIO> m_io;

    public:
        SurfaceUI() {}
        int          initialize(SurfaceRHI* rhi, PilotRenderer* prenderer, std::shared_ptr<SurfaceIO> pio);
        void         fontsUpload(SurfaceRHI* rhi);
        void         tick_pre(UIState* uistate);
        virtual void onTick(UIState* uistate) = 0;
        void         tick_post(UIState* uistate);
        virtual void registerInput() {};

        void draw_frame();
        int  clear();
        void setDefaultStyle();
    };

    class Surface
    {
    public:
        Surface(PilotRenderer* prenderer, const char* title = "Pilot", uint32_t width = 1280, uint32_t height = 768) :
            m_io(std::make_shared<SurfaceIO>(title, width, height)), m_rhi(std::make_shared<SurfaceRHI>(prenderer)),
            m_render(prenderer)
        {}
        int initialize(const FrameBuffer* framebuffer)
        {
            m_io->initialize();
            m_rhi->initialize(m_io.get(), framebuffer);
            return onInit();
        }
        bool setSurfaceUI(std::shared_ptr<SurfaceUI> pui)
        {
            m_ui = pui;
            m_ui->initialize(m_rhi.get(), m_render, m_io);
            return true;
        }
        bool tick(const FrameBuffer* framebuffer, UIState* uistate, SceneReleaseHandles& release_handles);
        int  clear()
        {
            int ret = onClear();
            // m_ui->clear();
            m_rhi->clear();
            m_io->clear();
            return ret;
        }

        virtual int onInit() { return 0; };
        virtual int onClear() { return 0; };

        std::shared_ptr<SurfaceIO> getSurfaceIO() { return m_io; }

        void   updateWindow(float pos_x, float pos_y, float width, float height) const;
        size_t updateCursorOnAxis(int axis_mode, const Vector2& cursor_uv, const Vector2& window_size) const;
        size_t getGuidOfPickedMesh(const Vector2& picked_uv) const;

    protected:
        std::shared_ptr<SurfaceIO>  m_io;
        std::shared_ptr<SurfaceRHI> m_rhi;
        std::shared_ptr<SurfaceUI>  m_ui;
        PilotRenderer*              m_render = nullptr;
    };

} // namespace Pilot
