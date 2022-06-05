#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/engine.h"

#include <fstream>
#include <sstream>
#include <string>

#include "_generated/serializer/all_serializer.h"

namespace Pilot
{
    void ConfigManager::initialize(const EngineInitParams& init_param)
    {
        m_root_folder = init_param.m_root_folder;
        // read configs
        std::ifstream     config_file(init_param.m_config_file_path);
        std::stringstream buffer;
        buffer << config_file.rdbuf();
        std::string config_file_text(buffer.str());

        std::string error;
        auto        config_json = PJson::parse(config_file_text, error);
        assert(error.empty());

        PSerializer::read(config_json, m_config);

        if (!m_config.asset_folder.empty())
        {
            m_asset_folder = m_root_folder / m_config.asset_folder;
        }
        if (!m_config.schema_folder.empty())
        {
            m_schema_folder = m_root_folder / m_config.schema_folder;
        }
        if (!m_config.big_icon.empty())
        {
            m_editor_big_icon_path = m_root_folder / m_config.big_icon;
        }
        if (!m_config.small_icon.empty())
        {
            m_editor_small_icon_path = m_root_folder / m_config.small_icon;
        }
        if (!m_config.font_file.empty())
        {
            m_editor_font_path = m_root_folder / m_config.font_file;
        }

        // set default value
        if (m_config.log_pattern.empty())
        {
            m_config.log_pattern = "[%^%l%$] %v";
        }

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        if (!m_config.jolt_asset_folder.empty())
        {
            m_jolt_physics_asset_folder = m_root_folder / jolt_asset_folder;
        }
#endif
    }

    const std::filesystem::path& ConfigManager::getRootFolder() const { return m_root_folder; }

    const std::filesystem::path& ConfigManager::getAssetFolder() const { return m_asset_folder; }

    const std::filesystem::path& ConfigManager::getSchemaFolder() const { return m_schema_folder; }

    const std::filesystem::path& ConfigManager::getEditorBigIconPath() const { return m_editor_big_icon_path; }

    const std::filesystem::path& ConfigManager::getEditorSmallIconPath() const { return m_editor_small_icon_path; }

    const std::filesystem::path& ConfigManager::getEditorFontPath() const { return m_editor_font_path; }

    const std::string& ConfigManager::getDefaultWorldUrl() const { return m_config.default_world_url; }

    const std::string& ConfigManager::getGlobalRenderingResUrl() const { return m_config.global_rendering_res; }

    const std::string& ConfigManager::getLogPattern() const { return m_config.log_pattern; }

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
    const std::filesystem::path& ConfigManager::getJoltPhysicsAssetFolder() const
    {
        return m_jolt_physics_asset_folder;
    }
#endif

} // namespace Pilot
