#pragma once

#include "runtime/core/base/public_singleton.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <unordered_set>

namespace Pilot
{
    class WindowSystem;
    class RenderSystem;

    extern bool g_is_editor_mode;
    extern std::unordered_set<std::string> g_editor_tick_component_types;

    struct EngineInitParams
    {
        std::filesystem::path m_root_folder;
        std::filesystem::path m_config_file_path;
    };

    class PilotEngine : public PublicSingleton<PilotEngine>
    {
        friend class PublicSingleton<PilotEngine>;
        friend class PilotEditor;

        static const float k_fps_alpha;

    protected:
        PilotEngine();

        bool                                  m_is_quit {false};
        std::chrono::steady_clock::time_point m_last_tick_time_point {std::chrono::steady_clock::now()};

        std::shared_ptr<WindowSystem> m_window_system;
        std::shared_ptr<RenderSystem> m_render_system;

        float m_average_duration {0.f};
        int   m_frame_count {0};
        int   m_fps {0};

        void logicalTick(float delta_time);
        bool rendererTick();

        void fps(float delta_time);

        /**
         *  Each frame can only be called once
         */
        float getDeltaTime();

    public:
        void startEngine(const EngineInitParams& param);
        void shutdownEngine();

        void initialize();
        void clear();

        bool isQuit() const { return m_is_quit; }
        void run();
        bool tickOneFrame(float delta_time);

        int getFPS() const { return m_fps; }

        std::shared_ptr<WindowSystem> getWindowSystem() const { return m_window_system; }
        std::shared_ptr<RenderSystem> getRenderSystem() const { return m_render_system; }
    };

} // namespace Pilot
