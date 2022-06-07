#pragma once

#include <filesystem>

#include "runtime/core/meta/reflection/reflection.h"

namespace Pilot
{
    struct EngineInitParams;

    REFLECTION_TYPE(Config)
    CLASS(Config, Fields)
    {
        REFLECTION_BODY(Config);

    public:
        std::string asset_folder;
        std::string schema_folder;
        std::string default_world_url;
        std::string big_icon;
        std::string small_icon;
        std::string font_file;
        std::string global_rendering_res;
        std::string log_pattern;

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        std::string jolt_asset_folder;
#endif
    };

    class ConfigManager
    {
    public:
        void initialize(const EngineInitParams& init_param);

        const std::filesystem::path& getRootFolder() const;
        const std::filesystem::path& getAssetFolder() const;
        const std::filesystem::path& getSchemaFolder() const;
        const std::filesystem::path& getEditorBigIconPath() const;
        const std::filesystem::path& getEditorSmallIconPath() const;
        const std::filesystem::path& getEditorFontPath() const;

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        const std::filesystem::path& getJoltPhysicsAssetFolder() const;
#endif

        const std::string& getDefaultWorldUrl() const;
        const std::string& getGlobalRenderingResUrl() const;
        const std::string& getLogPattern() const;

    private:
        std::filesystem::path m_root_folder;
        std::filesystem::path m_asset_folder;
        std::filesystem::path m_schema_folder;
        std::filesystem::path m_editor_big_icon_path;
        std::filesystem::path m_editor_small_icon_path;
        std::filesystem::path m_editor_font_path;

#ifdef ENABLE_PHYSICS_DEBUG_RENDERER
        std::filesystem::path m_jolt_physics_asset_folder;
#endif

        Config m_config;
    };
} // namespace Pilot
