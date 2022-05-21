#pragma once

#include "runtime/function/render/render_pass.h"

namespace Pilot
{
    class WindowUI;

    struct UIPassInitInfo : RenderPassInitInfo
    {
        VkRenderPass render_pass;
    };

    class UIPass : public RenderPass
    {
    public:
        virtual void initialize(const RenderPassInitInfo* init_info) override final;
        virtual void initializeUIRenderBackend(WindowUI* window_ui) override final;
        void         draw();

    private:
        WindowUI* m_window_ui;

    private:
        void uploadFonts();
    };
} // namespace Pilot
