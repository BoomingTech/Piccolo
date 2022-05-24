#include "runtime/function/global/global_context.h"

#include "core/log/log_system.h"

#include "runtime/engine.h"

#include "runtime/platform/file_service/file_service.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/engine.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/input/input_system.h"
#include "runtime/function/physics/physics_system.h"
#include "runtime/function/render/render_system.h"
#include "runtime/function/render/window_system.h"

namespace Pilot
{
    RuntimeGlobalContext g_runtime_global_context;

    void RuntimeGlobalContext::startSystems(const EngineInitParams& init_params)
    {
        m_config_manager = std::make_shared<ConfigManager>();
        m_config_manager->initialize(init_params);

        m_file_servcie = std::make_shared<FileService>();

        m_logger_system = std::make_shared<LogSystem>();

        m_asset_manager = std::make_shared<AssetManager>();

        m_physics_system = std::make_shared<PhysicsSystem>();

        m_world_manager = std::make_shared<WorldManager>();
        m_world_manager->initialize();

        m_window_system = std::make_shared<WindowSystem>();
        WindowCreateInfo window_create_info;
        m_window_system->initialize(window_create_info);

        m_input_system = std::make_shared<InputSystem>();
        m_input_system->initialize();

        m_render_system = std::make_shared<RenderSystem>();
        RenderSystemInitInfo render_init_info;
        render_init_info.window_system = m_window_system;
        m_render_system->initialize(render_init_info);

        MeshComponent::m_swap_context    = &(m_render_system->getSwapContext());
        CameraComponent::m_render_camera = &(*m_render_system->getRenderCamera());
    }

    void RuntimeGlobalContext::shutdownSystems()
    {
        m_render_system.reset();

        m_window_system.reset();

        m_scene_manager.reset();

        m_world_manager.reset();

        m_physics_system.reset();

        m_input_system.reset();

        m_asset_manager.reset();


        m_logger_system.reset();

        m_file_servcie.reset();

        m_config_manager.reset();
    }
} // namespace Pilot