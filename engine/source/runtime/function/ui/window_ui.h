#pragma once

#include <memory>

namespace Piccolo
{
    class WindowSystem;
    class RenderSystem;

    struct WindowUIInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;
        std::shared_ptr<RenderSystem> render_system;
    };

    class WindowUI
    {
    public:
        virtual void initialize(WindowUIInitInfo init_info) = 0;
        virtual void preRender() = 0;
    };
} // namespace Piccolo
