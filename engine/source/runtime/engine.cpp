#include "runtime/engine.h"

#include "runtime/core/base/macro.h"
#include "runtime/core/meta/reflection/reflection_register.h"

#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/input/input_system.h"
#include "runtime/function/particle/particle_manager.h"
#include "runtime/function/physics/physics_manager.h"
#include "runtime/function/render/render_system.h"
#include "runtime/function/render/window_system.h"
#include "runtime/function/render/debugdraw/debug_draw_manager.h"

namespace Piccolo
{
    bool                            g_is_editor_mode {false};
    std::unordered_set<std::string> g_editor_tick_component_types {};

    void PiccoloEngine::startEngine(const std::string& config_file_path)
    {
        Reflection::TypeMetaRegister::metaRegister();

        g_runtime_global_context.startSystems(config_file_path);

        LOG_INFO("engine start");
    }

    void PiccoloEngine::shutdownEngine()
    {
        LOG_INFO("engine shutdown");

        g_runtime_global_context.shutdownSystems();

        Reflection::TypeMetaRegister::metaUnregister();
    }

    void PiccoloEngine::initialize() {}
    void PiccoloEngine::clear() {}

    void PiccoloEngine::run()
    {
        std::shared_ptr<WindowSystem> window_system = g_runtime_global_context.m_window_system;
        ASSERT(window_system);

        while (!window_system->shouldClose())
        {
            const float delta_time = calculateDeltaTime();
            tickOneFrame(delta_time);
        }
    }

    float PiccoloEngine::calculateDeltaTime()
    {
        float delta_time;
        {
            using namespace std::chrono;

            steady_clock::time_point tick_time_point = steady_clock::now();
            duration<float> time_span = duration_cast<duration<float>>(tick_time_point - m_last_tick_time_point);
            delta_time                = time_span.count();

            m_last_tick_time_point = tick_time_point;
        }
        return delta_time;
    }

    bool PiccoloEngine::tickOneFrame(float delta_time)
    {
        logicalTick(delta_time);
        calculateFPS(delta_time);

        // single thread
        // exchange data between logic and render contexts
        g_runtime_global_context.m_render_system->swapLogicRenderData();

        rendererTick(delta_time);

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        g_runtime_global_context.m_physics_manager->renderPhysicsWorld(delta_time);
#endif

        g_runtime_global_context.m_window_system->pollEvents();


        g_runtime_global_context.m_window_system->setTitle(
            std::string("Piccolo - " + std::to_string(getFPS()) + " FPS").c_str());

        const bool should_window_close = g_runtime_global_context.m_window_system->shouldClose();
        return !should_window_close;
    }

    void PiccoloEngine::logicalTick(float delta_time)
    {
        g_runtime_global_context.m_world_manager->tick(delta_time);
        g_runtime_global_context.m_input_system->tick();
    }

    bool PiccoloEngine::rendererTick(float delta_time)
    {
        g_runtime_global_context.m_render_system->tick(delta_time);
        return true;
    }

    const float PiccoloEngine::s_fps_alpha = 1.f / 100;
    void        PiccoloEngine::calculateFPS(float delta_time)
    {
        m_frame_count++;

        if (m_frame_count == 1)
        {
            m_average_duration = delta_time;
        }
        else
        {
            m_average_duration = m_average_duration * (1 - s_fps_alpha) + delta_time * s_fps_alpha;
        }

        m_fps = static_cast<int>(1.f / m_average_duration);
    }
} // namespace Piccolo
