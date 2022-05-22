#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <unordered_set>

namespace Pilot
{
    extern bool                            g_is_editor_mode;
    extern std::unordered_set<std::string> g_editor_tick_component_types;

    struct EngineInitParams
    {
        std::filesystem::path m_root_folder;
        std::filesystem::path m_config_file_path;
    };

    class PilotEngine
    {
        friend class PilotEditor;

        static const float k_fps_alpha;

    public:
        void startEngine(const EngineInitParams& param);
        void shutdownEngine();

        void initialize();
        void clear();

        bool isQuit() const { return m_is_quit; }
        void run();
        bool tickOneFrame(float delta_time);

        int getFPS() const { return m_fps; }

    protected:
        void logicalTick(float delta_time);
        bool rendererTick();

        void calculateFPS(float delta_time);

        /**
         *  Each frame can only be called once
         */
        float calculateDeltaTime();

    protected:
        EngineInitParams m_init_params;

        bool m_is_quit {false};

        std::chrono::steady_clock::time_point m_last_tick_time_point {std::chrono::steady_clock::now()};

        float m_average_duration {0.f};
        int   m_frame_count {0};
        int   m_fps {0};
    };

} // namespace Pilot
