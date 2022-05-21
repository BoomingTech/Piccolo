#include "runtime/engine.h"

#include "runtime/core/base/macro.h"
#include "runtime/core/meta/reflection/reflection_register.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/input/input_system.h"

#include "runtime/function/render/render_system.h"
#include "runtime/function/render/window_system.h"

#include "runtime/function/ui/ui_system.h"

namespace Pilot
{
    bool                            g_is_editor_mode {false};
    std::unordered_set<std::string> g_editor_tick_component_types {};

    PilotEngine::PilotEngine()
    {
        m_window_system = std::make_shared<WindowSystem>();
        m_render_system = std::make_shared<RenderSystem>();
    }

    void PilotEngine::startEngine(const EngineInitParams& param)
    {
        Reflection::TypeMetaRegister::Register();

        ConfigManager::getInstance().initialize(param);
        AssetManager::getInstance().initialize();
        PUIManager::getInstance().initialize();
        InputSystem::getInstance().initialize();

        WorldManager::getInstance().initialize();

        WindowCreateInfo window_create_info;
        m_window_system->initialize(window_create_info);

        RenderSystemInitInfo render_init_info;
        render_init_info.window_system = m_window_system;
        m_render_system->initialize(render_init_info);

        MeshComponent::m_swap_context = &(m_render_system->getSwapContext());
        CameraComponent::m_render_camera = &(*m_render_system->getRenderCamera());

        LOG_INFO("engine start");
    }

    void PilotEngine::shutdownEngine()
    {
        LOG_INFO("engine shutdown");

        WorldManager::getInstance().clear();
        PUIManager::getInstance().clear();
        AssetManager::getInstance().clear();
        ConfigManager::getInstance().clear();

        Reflection::TypeMetaRegister::Unregister();
    }

    void PilotEngine::initialize() {}
    void PilotEngine::clear() {}
    void PilotEngine::run()
    {
        float delta_time;
        while (!m_window_system->shouldClose())
        {
            delta_time = getDeltaTime();

            logicalTick(delta_time);
            fps(delta_time);

            // single thread
            // exchange data between logic and render contexts
            m_render_system->swapLogicRenderData();

            rendererTick();

            m_window_system->pollEvents();

            m_window_system->setTile(std::string("Pilot - " + std::to_string(getFPS()) + " FPS").c_str());
        }
    }

    float PilotEngine::getDeltaTime()
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

    bool PilotEngine::tickOneFrame(float delta_time)
    {
        logicalTick(delta_time);
        fps(delta_time);

        // single thread
        // exchange data between logic and render contexts
        m_render_system->swapLogicRenderData();

        rendererTick();

        m_window_system->pollEvents();

        m_window_system->setTile(std::string("Pilot - " + std::to_string(getFPS()) + " FPS").c_str());

        return !m_window_system->shouldClose();
    }

    void PilotEngine::logicalTick(float delta_time)
    {
        WorldManager::getInstance().tick(delta_time);
        InputSystem::getInstance().tick();
        // PhysicsSystem::getInstance().tick(delta_time);
    }

    bool PilotEngine::rendererTick()
    {
        m_render_system->tick();
        return true;
    }

    const float PilotEngine::k_fps_alpha = 1.f / 100;
    void        PilotEngine::fps(float delta_time)
    {
        m_frame_count++;

        if (m_frame_count == 1)
        {
            m_average_duration = delta_time;
        }
        else
        {
            m_average_duration = m_average_duration * (1 - k_fps_alpha) + delta_time * k_fps_alpha;
        }

        m_fps = static_cast<int>(1.f / m_average_duration);
    }
} // namespace Pilot
