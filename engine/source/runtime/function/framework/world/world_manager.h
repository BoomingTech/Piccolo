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
        virtual ~WorldManager();

        WorldManager(const WorldManager&) = delete;
        WorldManager& operator=(const WorldManager&) = delete;

        void initialize();
        void clear();

        void reloadCurrentLevel();
        void saveCurrentLevel();
        void loadAsCurrentWorld(const std::string& world_url);
        void loadAsCurrentLevel(const std::string& level_url);

        void   tick(float delta_time);
        Level* getCurrentActiveLevel() const { return m_current_active_level; }

    protected:
        WorldManager() = default;

    private:
        void processPendingLoadWorld();
        void loadWorld(const std::filesystem::path& world_url);
        void loadLevel(const std::string& level_url);

        std::filesystem::path m_pending_load_world_url;
        std::filesystem::path m_pending_load_level_url;
        std::filesystem::path m_current_world_url;
        std::string           m_current_level_url;

        std::vector<Level*> m_levels;
        Level*              m_current_active_level {nullptr};
    };
} // namespace Pilot
