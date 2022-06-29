#pragma once
#include "common/precompiled.h"
class TemplateManager
{
public:
    static TemplateManager* getInstance()
    {
        static TemplateManager* m_pInstance;
        if (nullptr == m_pInstance)
            m_pInstance = new TemplateManager();
        return m_pInstance;
    }
    void loadTemplates(std::string path, std::string template_name);

    std::string renderByTemplate(std::string template_name, Mustache::data& template_data);

private:
    TemplateManager() {}
    TemplateManager(const TemplateManager&);
    TemplateManager&                             operator=(const TemplateManager&);
    std::unordered_map<std::string, std::string> m_template_pool;
};
