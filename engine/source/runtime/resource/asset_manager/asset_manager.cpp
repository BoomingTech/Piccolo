#include "runtime/resource/asset_manager/asset_manager.h"

#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/global/global_context.h"

namespace Pilot
{
    std::filesystem::path AssetManager::getFullPath(const std::string& relative_path) const
    {
        return g_runtime_global_context.m_config_manager->getRootFolder() / relative_path;
    }
} // namespace Pilot