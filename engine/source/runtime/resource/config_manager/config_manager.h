#pragma once

#include "runtime/engine.h"

#include <filesystem>

namespace Pilot
{
    class ConfigManager final : public PublicSingleton<ConfigManager>
    {
    public:
        ConfigManager(const ConfigManager&) = delete;
        ConfigManager& operator=(const ConfigManager&) = delete;

        //
        ConfigManager() = default;

    private:
        std::filesystem::path m_root_folder;
        std::filesystem::path m_asset_folder;
        std::filesystem::path m_schema_folder;
        std::filesystem::path m_default_world_path;
        std::filesystem::path m_editor_big_icon_path;
        std::filesystem::path m_editor_small_icon_path;
        std::filesystem::path m_editor_font_path;

    public:
        void initialize(const EngineInitParams& init_param);

        void clear();

        const std::filesystem::path& getRootFolder() const {return m_root_folder;}
        const std::filesystem::path& getAssetFolder() const {return m_asset_folder;}
        const std::filesystem::path& getSchemaFolder() const {return m_schema_folder;}
        const std::filesystem::path& getDefaultWorldPath() const {return m_default_world_path;}
        const std::filesystem::path& getEditorBigIconPath() const {return m_editor_big_icon_path;}
        const std::filesystem::path& getEditorSmallIconPath() const {return m_editor_small_icon_path;}
        const std::filesystem::path& getEditorFontPath() const {return m_editor_font_path;}
    };
} // namespace Pilot