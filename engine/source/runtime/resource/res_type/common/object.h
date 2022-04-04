#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include "runtime/core/math/transform.h"
#include <string>
#include <vector>
namespace Pilot
{
    REFLECTION_TYPE(ComponentDefinitionRes)
    CLASS(ComponentDefinitionRes, Fields)
    {
        REFLECTION_BODY(ComponentDefinitionRes);

    public:
        std::string m_type_name;
        std::string m_component;
    };

    REFLECTION_TYPE(ObjectDefinitionRes)
    CLASS(ObjectDefinitionRes, Fields)
    {
        REFLECTION_BODY(ObjectDefinitionRes);

    public:
        std::vector<std::string> m_components;
    };

    REFLECTION_TYPE(ObjectInstanceRes)
    CLASS(ObjectInstanceRes, Fields)
    {
        REFLECTION_BODY(ObjectInstanceRes);

    public:
        std::string              m_name;
        Transform                m_transform;
        std::string              m_definition;
        std::vector<std::string> m_instance_components;
    };
} // namespace Pilot
