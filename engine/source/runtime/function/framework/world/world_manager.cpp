#include "runtime/function/framework/world/world_manager.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/framework/level/level.h"

#include <unordered_set>

#include "_generated/serializer/all_serializer.h"

namespace Pilot
{
    WorldManager::~WorldManager() { clear(); }

    void WorldManager::initialize() { m_pending_load_world_url = ConfigManager::getInstance().getDefaultWorldUrl(); }

    void WorldManager::clear()
    {
        m_pending_load_world_url.clear();
        m_current_world_url.clear();
        m_current_level_url.clear();

        m_current_active_level = nullptr;
        for (Level* level : m_levels)
        {
            level->clear();
            delete level;
        }
        m_levels.clear();
    }

    void WorldManager::tick(float delta_time)
    {
        processPendingLoadWorld();

        for (Level* level : m_levels)
        {
            level->tickAll(delta_time);
        }
    }

    void WorldManager::processPendingLoadWorld()
    {
        if (m_pending_load_world_url.empty())
            return;

        std::filesystem::path pending_load_world_url = m_pending_load_world_url;
        clear();

        loadWorld(pending_load_world_url);
    }

    void WorldManager::loadWorld(const std::filesystem::path& world_url)
    {
        std::filesystem::path pending_load_world_path = ConfigManager::getInstance().getAssetFolder() / world_url;

        WorldRes world_res;
        AssetManager::getInstance().loadAsset(pending_load_world_path.generic_string(), world_res);

        // m_world = pending_load_world;
        m_current_world_url = world_url;

        typedef std::unordered_set<size_t> TypeIDSet;
        for (const std::string& level_url : world_res.m_levels)
        {
            loadLevel(level_url);
        }
    }

    void WorldManager::loadLevel(const std::string& level_url)
    {
        Level* level = new Level;
        level->load(level_url);
        m_levels.push_back(level);

        if (m_current_active_level == nullptr)
        {
            m_current_active_level = level;
            m_current_level_url    = level_url;
        }
    }

    void WorldManager::loadAsCurrentLevel(const std::string& level_url)
    {
        if (m_current_active_level != nullptr)
        {
            auto iter = m_levels.begin();
            while (iter != m_levels.end())
            {
                if (*iter == m_current_active_level)
                    break;
                ++iter;
            }
            m_levels.erase(iter);

            m_current_active_level->clear();
            delete m_current_active_level;
            m_current_active_level = nullptr;
        }

        loadLevel(level_url);
    }

    void WorldManager::loadAsCurrentWorld(const std::string& world_url) { m_pending_load_world_url = world_url; }

    void WorldManager::reloadCurrentLevel()
    {
        if (m_current_active_level == nullptr)
            return;

        std::string current_level_url = m_current_active_level->getLevelResUrl();
        loadAsCurrentLevel(current_level_url);
    }

    void WorldManager::saveCurrentLevel()
    {
        if (m_current_active_level == nullptr)
            return;

        m_current_active_level->save();
    }
} // namespace Pilot
