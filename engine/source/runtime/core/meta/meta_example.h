#pragma once

#include "runtime/core/meta/reflection/reflection.h"

namespace Piccolo
{
    REFLECTION_TYPE(BaseTest)
    CLASS(BaseTest, Fields)
    {
        REFLECTION_BODY(BaseTest);

    public:
        int               m_int;
        std::vector<int*> m_int_vector;
    };

    REFLECTION_TYPE(Test1)
    CLASS(Test1 : public BaseTest, WhiteListFields)
    {
        REFLECTION_BODY(Test1);

    public:
        META(Enable)
        char m_char;
    };

    REFLECTION_TYPE(Test2)
    CLASS(Test2 : public BaseTest, , Fields)
    {
        REFLECTION_BODY(Test2);

    public:
        std::vector<Reflection::ReflectionPtr<BaseTest>> m_test_base_array;
    };
} // namespace Piccolo
