#pragma once

#include "runtime/engine.h"

#include <filesystem>

namespace Pilot
{
    class ConfigManager final : public PublicSingleton<ConfigManager>
    {
        friend class PublicSingleton<ConfigManager>;

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

        const std::filesystem::path& getRootFolder() const;
        const std::filesystem::path& getAssetFolder() const;
        const std::filesystem::path& getSchemaFolder() const;
        const std::filesystem::path& getDefaultWorldPath() const;
        const std::filesystem::path& getEditorBigIconPath() const;
        const std::filesystem::path& getEditorSmallIconPath() const;
        const std::filesystem::path& getEditorFontPath() const;
    };
} // namespace Pilot