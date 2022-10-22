#pragma once

#include "runtime/resource/res_type/common/world.h"

#include <filesystem>
#include <string>

namespace Piccolo
{
    class Level;
    class LevelDebugger;
    class PhysicsScene;

    /// Manage all game worlds, it should be support multiple worlds, including game world and editor world.
    /// Currently, the implement just supports one active world and one active level
    class WorldManager
    {
    public:
        virtual ~WorldManager();

        void initialize();
        void clear();

        void reloadCurrentLevel();
        void saveCurrentLevel();

        void                 tick(float delta_time);
        std::weak_ptr<Level> getCurrentActiveLevel() const { return m_current_active_level; }

        std::weak_ptr<PhysicsScene> getCurrentActivePhysicsScene() const;

    private:
        bool loadWorld(const std::string& world_url);
        bool loadLevel(const std::string& level_url);

        bool                      m_is_world_loaded {false};
        std::string               m_current_world_url;
        std::shared_ptr<WorldRes> m_current_world_resource;

        // all loaded levels, key: level url, vaule: level instance
        std::unordered_map<std::string, std::shared_ptr<Level>> m_loaded_levels;
        // active level, currently we just support one active level
        std::weak_ptr<Level> m_current_active_level;

        //debug level
        std::shared_ptr<LevelDebugger> m_level_debugger;
    };
} // namespace Piccolo
