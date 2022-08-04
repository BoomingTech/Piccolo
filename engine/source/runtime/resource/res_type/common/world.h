#pragma once
#include "runtime/core/meta/reflection/reflection.h"
#include <string>
#include <vector>
namespace Piccolo
{
    REFLECTION_TYPE(WorldRes)
    CLASS(WorldRes, Fields)
    {
        REFLECTION_BODY(WorldRes);

    public:
        // world name
        std::string              m_name;

        // all level urls for this world
        std::vector<std::string> m_level_urls;

        // the default level for this world, which should be first loading level
        std::string m_default_level_url;
    };
} // namespace Piccolo
