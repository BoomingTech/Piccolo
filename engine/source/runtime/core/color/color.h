#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include "runtime/core/math/vector3.h"

namespace Piccolo
{
    REFLECTION_TYPE(Color)
    CLASS(Color, Fields)
    {
        REFLECTION_BODY(Color);

    public:
        float r;
        float g;
        float b;

        Vector3 toVector3() const { return Vector3(r, g, b); }
    };
}