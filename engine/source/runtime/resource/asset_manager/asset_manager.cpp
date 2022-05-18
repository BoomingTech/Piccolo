#include "runtime/resource/asset_manager/asset_manager.h"

#include "runtime/resource/config_manager/config_manager.h"

namespace Pilot
{
    void AssetManager::initialize()
    {
    }

    std::filesystem::path AssetManager::getFullPath(const std::string& relative_path) const
    {
        return ConfigManager::getInstance().getRootFolder() / relative_path;
    }
} // namespace Pilot