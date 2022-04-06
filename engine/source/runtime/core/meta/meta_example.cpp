#include "runtime/core/meta/meta_example.h"
#include "_generated/serializer/all_serializer.h"

#include "runtime/core/base/macro.h"

#include <filesystem>
#include <fstream>
#include <iostream>
namespace Pilot
{
    void metaExample()
    {
        Test1 test1_in;
        test1_in.m_int  = 12;
        test1_in.m_char = 'g';
        int i           = 1;
        test1_in.m_int_vector.emplace_back(&i);

        Test1 test1_out;
        // test on array
        Test2 test2_in;
        test2_in.m_test_base_array.emplace_back("Test1", &test1_in);
        Test1 Test2_temp;
        test2_in.m_test_base_array.emplace_back("Test1", &Test2_temp);

        // serializer & deserializer

        // write Test1_in (object) to Test1_json_in (json)
        auto test1_json_in = PSerializer::write(test1_in);

        std::string test1_context = test1_json_in.dump();

        // read Test1_context (json) to Test1_out (object)
        std::string err;

        auto&& Test1_json = PJson::parse(test1_context, err);
        PSerializer::read(Test1_json, test1_out);
        LOG_INFO(test1_context);

        auto        Test2_json_in = PSerializer::write(test2_in);
        std::string test2_context = Test2_json_in.dump();

        std::fstream out_put("out.txt", std::ios::out);
        out_put << test2_context;
        out_put.flush();
        out_put.close();

        Test2  test2_out;
        auto&& test2_json = PJson::parse(test2_context, err);
        PSerializer::read(test2_json, test2_out);
        LOG_INFO(test2_context.c_str());

        // reflection
        auto                       meta = TypeMetaDef(Test2, &test2_out);
        Reflection::FieldAccessor* fields;
        int                        fields_count = meta.m_meta.getFieldsList(fields);
        for (int i = 0; i < fields_count; ++i)
        {
            auto field_accesser = fields[i];
            std::cout << field_accesser.getFieldTypeName() << " " << field_accesser.getFieldName() << " "
                      << (char*)field_accesser.get(meta.m_instance) << std::endl;
            if (field_accesser.isArrayType())
            {
                Reflection::ArrayAccessor array_accesser;
                if (Reflection::TypeMeta::newArrayAccessorFromName(field_accesser.getFieldTypeName(), array_accesser))
                {
                    void* field_instance = field_accesser.get(meta.m_instance);
                    int   count          = array_accesser.getSize(field_instance);
                    auto  typeMetaItem   = Reflection::TypeMeta::newMetaFromName(array_accesser.getElementTypeName());
                    for (int index = 0; index < count; ++index)
                    {
                        std::cout << ":L:" << index << ":R:" << (int*)array_accesser.get(index, field_instance)
                                  << std::endl;
                        ;
                    }
                }
            }
        }
    }
} // namespace Pilot
