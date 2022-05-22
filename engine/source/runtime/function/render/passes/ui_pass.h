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
        void initialize(const RenderPassInitInfo* init_info) override final;
        void initializeUIRenderBackend(WindowUI* window_ui) override final;
        void draw() override final;

    private:
        void uploadFonts();

    private:
        WindowUI* m_window_ui;
    };
} // namespace Pilot
