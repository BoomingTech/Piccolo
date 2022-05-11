#include "runtime/function/framework/object/object.h"

#include "runtime/engine.h"
#include "runtime/function/framework/component/component.h"
#include "runtime/function/framework/component/transform/transform_component.h"

#include "runtime/resource/asset_manager/asset_manager.h"

#include <cassert>
#include <unordered_set>

#include "_generated/serializer/all_serializer.h"

namespace Pilot
{
    GObject::~GObject()
    {
        for (auto& component : m_components)
        {
            PILOT_REFLECTION_DELETE(component);
        }
        m_components.clear();
    }

    void GObject::tick(float delta_time)
    {
        for (auto& component : m_components)
        {
            if (component->m_tick_in_editor_mode == false && g_is_editor_mode)
            {
                continue;
            }
            component->tick(delta_time);
        }
    }

    bool GObject::load(const ObjectInstanceRes& object_instance_res)
    {
        setName(object_instance_res.m_name);

        // load transform component
        auto transform_component_ptr = PILOT_REFLECTION_NEW(TransformComponent, object_instance_res.m_transform, this);
        transform_component_ptr->m_tick_in_editor_mode = true;
        m_components.push_back(transform_component_ptr);
        m_component_type_names.push_back("TransformComponent");

        // load object instance components
        TypeNameSet instance_component_type_set;
        if (loadComponents(object_instance_res.m_instance_components, instance_component_type_set) == false)
            return false;

        // load object definition components
        m_definition_url = object_instance_res.m_definition;

        ObjectDefinitionRes definition_res;
        AssetManager::getInstance().loadAsset(m_definition_url, definition_res);

        if (loadComponents(definition_res.m_components, instance_component_type_set) == false)
            return false;

        return true;
    }

    void GObject::save(ObjectInstanceRes& out_object_instance_res)
    {
        out_object_instance_res.m_name       = m_name;
        out_object_instance_res.m_definition = m_definition_url;

        const TransformComponent* transform_conponent = tryGetComponentConst(TransformComponent);
        out_object_instance_res.m_transform           = transform_conponent->getTransformConst();
    }

    bool GObject::loadComponents(const std::vector<std::string>& components,
                                 TypeNameSet&                    out_instance_component_type_set)
    {
        AssetManager&          asset_manager = AssetManager::getInstance();
        ComponentDefinitionRes definition_res;

        for (const std::string& definition_res_url : components)
        {
            asset_manager.loadAsset(definition_res_url, definition_res);
            if (loadComponentDefinition(definition_res, false, out_instance_component_type_set) == false)
                return false;
        }

        return true;
    }

    bool GObject::loadComponentDefinition(const ComponentDefinitionRes& component_definition_res,
                                          const bool                    is_instance_component,
                                          TypeNameSet&                  out_instance_component_type_set)
    {
        AssetManager&          asset_manager = AssetManager::getInstance();
        ComponentDefinitionRes component_definition;

        if (is_instance_component || out_instance_component_type_set.count(component_definition_res.m_type_name) == 0)
        {
            auto&& component_loader = asset_manager.getComponentLoader(component_definition_res.m_type_name);
            auto&& component        = component_loader(component_definition_res.m_component, this);

            if (component)
            {
                m_components.push_back(component);
                m_component_type_names.push_back(component_definition_res.m_type_name);
                out_instance_component_type_set.insert(component_definition_res.m_type_name);
            }
            else
            {
                return false;
            }
        }

        return true;
    }

} // namespace Pilot