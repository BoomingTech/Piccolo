#pragma once

#include "runtime/core/base/public_singleton.h"
#include "runtime/resource/res_type/common/world.h"

#include <filesystem>
#include <string>

namespace Pilot
{
    class PWorldT;
    class Level;

    class WorldManager : public PublicSingleton<WorldManager>
    {
        friend class PublicSingleton<WorldManager>;

    public:
        WorldManager() {}
        ~WorldManager();

        WorldManager(const WorldManager&) = delete;
        WorldManager& operator=(const WorldManager&) = delete;

        void initialize();
        void clear();

        void reloadCurrentLevel();
        void saveCurrentLevel();

        void   tick(float delta_time);
        Level* getCurrentActiveLevel() const { return m_current_active_level; }

    private:
        void processPendingLoadWorld();
        void loadWorld(const WorldRes& pending_load_world);
        void loadLevel(const std::string& level_url);

        std::filesystem::path m_pending_load_world_path;
        std::string           m_current_world_name;

        std::vector<Level*> m_levels;
        Level*              m_current_active_level {nullptr};
    };
} // namespace Pilot
