#pragma once
#include "runtime/function/render/include/render/framebuffer.h"

namespace Pilot
{
    class EditorTranslationAxis : public RenderMesh
    {
    public:
        EditorTranslationAxis();
    };

    class EditorRotationAxis : public RenderMesh
    {
    public:
        EditorRotationAxis();
    };

    class EditorScaleAxis : public RenderMesh
    {
    public:
        EditorScaleAxis();
    };
} // namespace Pilot
