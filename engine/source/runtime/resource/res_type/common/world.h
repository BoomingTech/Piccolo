#pragma once
#include "runtime/core/meta/reflection/reflection.h"
#include <string>
#include <vector>
namespace Pilot
{
    REFLECTION_TYPE(WorldRes)
    CLASS(WorldRes, Fields)
    {
        REFLECTION_BODY(WorldRes);

    public:
        std::string              m_name;
        std::vector<std::string> m_levels;
    };
} // namespace Pilot
