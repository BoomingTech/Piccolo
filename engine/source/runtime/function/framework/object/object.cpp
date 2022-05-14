#include "runtime/function/framework/object/object.h"

#include "runtime/core/meta/reflection/reflection.h"
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

    bool GObject::hasComponent(const std::string& compenent_type_name) const
    {
        for (const auto& component : m_components)
        {
            if (component.getTypeName() == compenent_type_name)
                return true;
        }

        return false;
    }

    bool GObject::load(const ObjectInstanceRes& object_instance_res)
    {
        m_components.clear();

        setName(object_instance_res.m_name);

        // load object instance components
        m_components = object_instance_res.m_new_components;

        // load transform component
        if (hasComponent("TransformComponent") == false)
        {
            auto transform_component_ptr =
                PILOT_REFLECTION_NEW(TransformComponent, object_instance_res.m_transform, this);
            transform_component_ptr->m_tick_in_editor_mode = true;
            m_components.push_back(transform_component_ptr);
        }

        // load object definition components
        m_definition_url = object_instance_res.m_definition;

        ObjectDefinitionRes definition_res;

        const bool is_loaded_success = AssetManager::getInstance().loadAsset(m_definition_url, definition_res);
        if (!is_loaded_success)
            return false;

        for (auto loaded_component : definition_res.m_components)
        {
            const std::string type_name = loaded_component.getTypeName();
            // don't create component if it has been instanced
            if (hasComponent(type_name))
                continue;

            loaded_component->postLoadResource(this);

            m_components.push_back(loaded_component);
        }

        return true;
    }

    void GObject::save(ObjectInstanceRes& out_object_instance_res)
    {
        out_object_instance_res.m_name       = m_name;
        out_object_instance_res.m_definition = m_definition_url;

        const TransformComponent* transform_conponent = tryGetComponentConst(TransformComponent);
        out_object_instance_res.m_transform           = transform_conponent->getTransformConst();

        out_object_instance_res.m_new_components = m_components;
    }

} // namespace Pilot