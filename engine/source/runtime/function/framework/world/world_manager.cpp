#include "runtime/function/framework/world/world_manager.h"

#include "runtime/core/base/macro.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/framework/level/level.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/framework/level/level_debugger.h"

#include "_generated/serializer/all_serializer.h"

namespace Piccolo
{
    WorldManager::~WorldManager() { clear(); }

    void WorldManager::initialize()
    {
        m_is_world_loaded   = false;
        m_current_world_url = g_runtime_global_context.m_config_manager->getDefaultWorldUrl();

        //debugger
        m_level_debugger = std::make_shared<LevelDebugger>();
    }

    void WorldManager::clear()
    {
        // unload all loaded levels
        for (auto level_pair : m_loaded_levels)
        {
            level_pair.second->unload();
        }
        m_loaded_levels.clear();

        m_current_active_level.reset();

        // clear world
        m_current_world_resource.reset();
        m_current_world_url.clear();
        m_is_world_loaded = false;

        //clear debugger
        m_level_debugger.reset();
    }

    void WorldManager::tick(float delta_time)
    {
        if (!m_is_world_loaded)
        {
            loadWorld(m_current_world_url);
        }

        // tick the active level
        std::shared_ptr<Level> active_level = m_current_active_level.lock();
        if (active_level)
        {
            active_level->tick(delta_time);
            m_level_debugger->tick(active_level);
        }
    }

    std::weak_ptr<PhysicsScene> WorldManager::getCurrentActivePhysicsScene() const
    {
        std::shared_ptr<Level> active_level = m_current_active_level.lock();
        if (!active_level)
        {
            return std::weak_ptr<PhysicsScene>();
        }

        return active_level->getPhysicsScene();
    }

    bool WorldManager::loadWorld(const std::string& world_url)
    {
        LOG_INFO("loading world: {}", world_url);
        WorldRes   world_res;
        const bool is_world_load_success = g_runtime_global_context.m_asset_manager->loadAsset(world_url, world_res);
        if (!is_world_load_success)
        {
            return false;
        }

        m_current_world_resource = std::make_shared<WorldRes>(world_res);

        const bool is_level_load_success = loadLevel(world_res.m_default_level_url);
        if (!is_level_load_success)
        {
            return false;
        }

        // set the default level to be active level
        auto iter = m_loaded_levels.find(world_res.m_default_level_url);
        ASSERT(iter != m_loaded_levels.end());

        m_current_active_level = iter->second;

        m_is_world_loaded = true;

        LOG_INFO("world load succeed!");
        return true;
    }

    bool WorldManager::loadLevel(const std::string& level_url)
    {
        std::shared_ptr<Level> level = std::make_shared<Level>();
        // set current level temporary
        m_current_active_level       = level;

        const bool is_level_load_success = level->load(level_url);
        if (is_level_load_success == false)
        {
            return false;
        }

        m_loaded_levels.emplace(level_url, level);

        return true;
    }

    void WorldManager::reloadCurrentLevel()
    {
        auto active_level = m_current_active_level.lock();
        if (active_level == nullptr)
        {
            LOG_WARN("current level is nil");
            return;
        }

        const std::string level_url = active_level->getLevelResUrl();
        active_level->unload();
        m_loaded_levels.erase(level_url);

        const bool is_load_success = loadLevel(level_url);
        if (!is_load_success)
        {
            LOG_ERROR("load level failed {}", level_url);
            return;
        }

        // update the active level instance
        auto iter = m_loaded_levels.find(level_url);
        ASSERT(iter != m_loaded_levels.end());

        m_current_active_level = iter->second;

        LOG_INFO("reload current evel succeed");
    }

    void WorldManager::saveCurrentLevel()
    {
        auto active_level = m_current_active_level.lock();

        if (active_level == nullptr)
        {
            LOG_ERROR("save level failed, no active level");
            return;
        }

        active_level->save();
    }
} // namespace Piccolo
