#pragma once

#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/resource/res_type/data/blend_state.h"

#include <string>
#include <vector>

namespace Piccolo
{

    REFLECTION_TYPE(AnimationResultElement)
    CLASS(AnimationResultElement, WhiteListFields)
    {
        REFLECTION_BODY(AnimationResultElement);

    public:
        int        index;
        Matrix4x4_ transform;
    };

    REFLECTION_TYPE(AnimationResult)
    CLASS(AnimationResult, Fields)
    {
        REFLECTION_BODY(AnimationResult);

    public:
        std::vector<AnimationResultElement> node;
    };

    REFLECTION_TYPE(AnimationComponentRes)
    CLASS(AnimationComponentRes, Fields)
    {
        REFLECTION_BODY(AnimationComponentRes);

    public:
        std::string skeleton_file_path;
        BlendState  blend_state;
        // animation to skeleton map
        float       frame_position; // 0-1

        META(Disable)
        AnimationResult animation_result;
    };

} // namespace Piccolo