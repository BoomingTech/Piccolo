#pragma once

#include "runtime/core/base/public_singleton.h"
#include "runtime/core/meta/serializer/serializer.h"

#include "runtime/function/framework/component/component.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

#include "_generated/serializer/all_serializer.h"

namespace Pilot
{
    class AssetManager : public PublicSingleton<AssetManager>
    {
    public:
        template<typename AssetType>
        void loadAsset(const std::string& asset_url, AssetType& out_asset) const
        {
            // read json file to string
            std::ifstream     asset_json_file(getFullPath(asset_url));
            std::stringstream buffer;
            buffer << asset_json_file.rdbuf();
            std::string asset_json_text(buffer.str());

            // parse to json object and read to runtime res object
            std::string error;
            auto&&      asset_json = PJson::parse(asset_json_text, error);
            assert(error.empty());

            PSerializer::read(asset_json, out_asset);
        }

        template<typename AssetType>
        void saveAsset(const AssetType& out_asset, const std::string& asset_url) const
        {
            // write to json object and dump to string
            auto&&        asset_json      = PSerializer::write(out_asset);
            std::string&& asset_json_text = asset_json.dump();

            // write to file
            std::ofstream asset_json_file(getFullPath(asset_url));
            asset_json_file << asset_json_text;
            asset_json_file.flush();
        }

        void initialize();
        void clear() {}

        std::filesystem::path getFullPath(const std::string& relative_path) const;

        typedef std::function<Reflection::ReflectionPtr<Component>(std::string, GObject*)> ComponentLoaderFunc;
        ComponentLoaderFunc getComponentLoader(std::string component_type_name)
        {
            return m_loader_map[component_type_name];
        }

        void registerComponentType(std::string component_type_name, ComponentLoaderFunc func)
        {
            m_loader_map[component_type_name] = func;
        }

#define REGISTER_COMPONENT(COMPONENT_TYPE, COMPONENT_RES_TYPE, TICK_IN_EDITOR_MODE) \
    registerComponentType(#COMPONENT_TYPE, [this](std::string component_res_url, GObject* parent_object) { \
        COMPONENT_RES_TYPE component_res; \
        loadAsset(component_res_url, component_res); \
        auto component                   = PILOT_REFLECTION_NEW(COMPONENT_TYPE, component_res, parent_object); \
        component->m_tick_in_editor_mode = TICK_IN_EDITOR_MODE; \
        return component; \
    });

    private:
        std::unordered_map<std::string, ComponentLoaderFunc> m_loader_map;
    };
} // namespace Pilot
