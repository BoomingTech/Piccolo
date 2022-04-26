#pragma once

#include "runtime/function/render/include/render/framebuffer.h"
#include "runtime/function/render/include/render/surface_io.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_manager.h"

#include <functional>

namespace Pilot
{
    class PilotRenderer;
    class PVulkanManager;
    class SurfaceUI;

    class SurfaceRHI
    {
        PilotRenderer* m_pilot_renderer;
        PRenderPath    m_curpath {PRenderPath::Clustered};

    public:
        GLFWwindow*     m_window;
        PVulkanManager* m_vulkan_manager;

        SurfaceRHI(PilotRenderer* prenderer) : m_pilot_renderer(prenderer) {}
        ~SurfaceRHI() { delete m_vulkan_manager; }
        void initialize(SurfaceIO* io, const FrameBuffer* framebuffer);
        void initializeUI(SurfaceUI* ui);
        void tick(const FrameBuffer* framebuffer, SceneReleaseHandles& release_handles, UIState* uistate);
        void clear();
    };

    class SurfaceUI
    {
    public:
        std::shared_ptr<SurfaceIO> m_io;
        void                       initialize(std::shared_ptr<SurfaceIO> pio);

    public:
        inline SurfaceUI() : m_tmp_uistate(NULL) {}
        UIState*     m_tmp_uistate;
        virtual void onTick(UIState* uistate) = 0;
        virtual void registerInput()          = 0;
        float        getContentScale() const;

    protected:
        float getIndentScale() const;
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
            m_ui->initialize(m_io);
            m_rhi->initializeUI(m_ui.get());
            return true;
        }
        bool tick(const FrameBuffer* framebuffer, UIState* uistate, SceneReleaseHandles& release_handles);
        int  clear()
        {
            int ret = onClear();
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
