#pragma once
#include "common/schema_module.h"

#include <functional>
#include <string>
namespace Generator
{
    class GeneratorInterface
    {
    public:
        GeneratorInterface(std::string                             out_path,
                           std::string                             root_path,
                           std::function<std::string(std::string)> get_include_func) :
            m_out_path(out_path),
            m_root_path(root_path), m_get_include_func(get_include_func)
        {}
        virtual int  generate(std::string path, SchemaMoudle schema) = 0;
        virtual void finish() {};

        virtual ~GeneratorInterface() {};

    protected:
        virtual void prepareStatus(std::string path);
        virtual void genClassRenderData(std::shared_ptr<Class> class_temp, Mustache::data& class_def);
        virtual void genClassFieldRenderData(std::shared_ptr<Class> class_temp, Mustache::data& feild_defs);
        virtual void genClassMethodRenderData(std::shared_ptr<Class> class_temp, Mustache::data& method_defs);

        virtual std::string processFileName(std::string path) = 0;

        std::string                             m_out_path {"gen_src"};
        std::string                             m_root_path;
        std::function<std::string(std::string)> m_get_include_func;
    };
} // namespace Generator
