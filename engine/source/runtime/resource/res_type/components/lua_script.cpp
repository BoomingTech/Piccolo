#include "runtime/resource/res_type/components/lua_script.h"
#include "runtime/core/base/macro.h"
#include "runtime/function/global/global_context.h"
#include "runtime/resource/config_manager/config_manager.h"

#include <fstream>

namespace Piccolo
{
    void LuaScriptRes::loadScriptContent()
    {
        // TODO : use file service
        auto file_dir = g_runtime_global_context.m_config_manager->getAssetFolder();
        file_dir      = file_dir / m_script_url;

        LOG_DEBUG("open lua script: " + file_dir.generic_string());

        std::ifstream fin;
        std::string   temp;
        m_script_content = "";
        fin.open(file_dir, std::ios::in);
        while (std::getline(fin, temp))
        {
            m_script_content += temp;
            m_script_content += "\n";
        }
        fin.close();

        LOG_DEBUG("script content:\n" + m_script_content);
    }
} // namespace Piccolo
