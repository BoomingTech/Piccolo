#pragma once

#include <filesystem>

namespace Pilot
{
    struct EngineInitParams;

    class ConfigManager
    {
    public:
        void initialize(const EngineInitParams& init_param);
        void clear();

        const std::filesystem::path& getRootFolder() const;
        const std::filesystem::path& getAssetFolder() const;
        const std::filesystem::path& getSchemaFolder() const;
        const std::filesystem::path& getEditorBigIconPath() const;
        const std::filesystem::path& getEditorSmallIconPath() const;
        const std::filesystem::path& getEditorFontPath() const;

        const std::string& getDefaultWorldUrl() const;
        const std::string& getGlobalRenderingResUrl() const;

    private:
        std::filesystem::path m_root_folder;
        std::filesystem::path m_asset_folder;
        std::filesystem::path m_schema_folder;
        std::filesystem::path m_editor_big_icon_path;
        std::filesystem::path m_editor_small_icon_path;
        std::filesystem::path m_editor_font_path;

        std::string m_default_world_url;
        std::string m_global_rendering_res_url;
    };
} // namespace Pilot
