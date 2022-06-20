#pragma once
namespace Piccolo
{
    namespace Reflection
    {
        class TypeMetaRegister
        {
        public:
            static void Register();
            static void Unregister();
        };
    } // namespace Reflection
} // namespace Piccolo