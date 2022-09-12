#pragma once

#include <filesystem>

namespace Piccolo
{
    struct EngineInitParams;

    class ConfigManager
    {
    public:
        void initialize(const std::filesystem::path& config_file_path);

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
        const std::string& getGlobalParticleResUrl() const;

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

        std::string m_default_world_url;
        std::string m_global_rendering_res_url;
        std::string m_global_particle_res_url;
    };
} // namespace Piccolo
