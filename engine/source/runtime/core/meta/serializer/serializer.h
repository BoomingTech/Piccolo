#pragma once
#include "runtime/core/meta/json.h"
#include "runtime/core/meta/reflection/reflection.h"

#include <cassert>

namespace Piccolo
{
    template<typename...>
    inline constexpr bool always_false = false;

    class Serializer
    {
    public:
        template<typename T>
        static Json writePointer(T* instance)
        {
            return Json::object {{"$typeName", Json {"*"}}, {"$context", Serializer::write(*instance)}};
        }

        template<typename T>
        static T*& readPointer(const Json& json_context, T*& instance)
        {
            assert(instance == nullptr);
            std::string type_name = json_context["$typeName"].string_value();
            assert(!type_name.empty());
            if ('*' == type_name[0])
            {
                instance = new T;
                read(json_context["$context"], *instance);
            }
            else
            {
                instance = static_cast<T*>(
                    Reflection::TypeMeta::newFromNameAndJson(type_name, json_context["$context"]).m_instance);
            }
            return instance;
        }

        template<typename T>
        static Json write(const Reflection::ReflectionPtr<T>& instance)
        {
            T*          instance_ptr = static_cast<T*>(instance.operator->());
            std::string type_name    = instance.getTypeName();
            return Json::object {{"$typeName", Json(type_name)},
                                  {"$context", Reflection::TypeMeta::writeByName(type_name, instance_ptr)}};
        }

        template<typename T>
        static T*& read(const Json& json_context, Reflection::ReflectionPtr<T>& instance)
        {
            std::string type_name = json_context["$typeName"].string_value();
            instance.setTypeName(type_name);
            return readPointer(json_context, instance.getPtrReference());
        }

        template<typename T>
        static Json write(const T& instance)
        {

            if constexpr (std::is_pointer<T>::value)
            {
                return writePointer((T)instance);
            }
            else
            {
                static_assert(always_false<T>, "Serializer::write<T> has not been implemented yet!");
                return Json();
            }
        }

        template<typename T>
        static T& read(const Json& json_context, T& instance)
        {
            if constexpr (std::is_pointer<T>::value)
            {
                return readPointer(json_context, instance);
            }
            else
            {
                static_assert(always_false<T>, "Serializer::read<T> has not been implemented yet!");
                return instance;
            }
        }
    };

    // implementation of base types
    template<>
    Json Serializer::write(const char& instance);
    template<>
    char& Serializer::read(const Json& json_context, char& instance);

    template<>
    Json Serializer::write(const int& instance);
    template<>
    int& Serializer::read(const Json& json_context, int& instance);

    template<>
    Json Serializer::write(const unsigned int& instance);
    template<>
    unsigned int& Serializer::read(const Json& json_context, unsigned int& instance);

    template<>
    Json Serializer::write(const float& instance);
    template<>
    float& Serializer::read(const Json& json_context, float& instance);

    template<>
    Json Serializer::write(const double& instance);
    template<>
    double& Serializer::read(const Json& json_context, double& instance);

    template<>
    Json Serializer::write(const bool& instance);
    template<>
    bool& Serializer::read(const Json& json_context, bool& instance);

    template<>
    Json Serializer::write(const std::string& instance);
    template<>
    std::string& Serializer::read(const Json& json_context, std::string& instance);

    // template<>
    // Json Serializer::write(const Reflection::object& instance);
    // template<>
    // Reflection::object& Serializer::read(const Json& json_context, Reflection::object& instance);

    ////////////////////////////////////
    ////sample of generation coder
    ////////////////////////////////////
    // class test_class
    //{
    // public:
    //     int a;
    //     unsigned int b;
    //     std::vector<int> c;
    // };
    // class ss;
    // class jkj;
    // template<>
    // Json Serializer::write(const ss& instance);
    // template<>
    // Json Serializer::write(const jkj& instance);

    /*REFLECTION_TYPE(jkj)
    CLASS(jkj,Fields)
    {
        REFLECTION_BODY(jkj);
        int jl;
    };

    REFLECTION_TYPE(ss)
    CLASS(ss:public jkj,WhiteListFields)
    {
        REFLECTION_BODY(ss);
        int jl;
    };*/

    ////////////////////////////////////
    ////template of generation coder
    ////////////////////////////////////
    // template<>
    // Json Serializer::write(const test_class& instance);
    // template<>
    // test_class& Serializer::read(const Json& json_context, test_class& instance);

    //
    ////////////////////////////////////
} // namespace Piccolo
