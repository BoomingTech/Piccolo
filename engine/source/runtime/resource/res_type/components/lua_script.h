#pragma once
#include "runtime/core/meta/reflection/reflection.h"

namespace Piccolo
{
    REFLECTION_TYPE(LuaScriptRes)
    CLASS(LuaScriptRes, WhiteListFields)
    {
        REFLECTION_BODY(LuaScriptRes);

    public:
        void loadScriptContent();

    public:
        META(Enable)
        std::string m_script_url;

        std::string m_script_content;
    };
} // namespace Piccolo
