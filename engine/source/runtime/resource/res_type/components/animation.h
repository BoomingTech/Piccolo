#pragma once

#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/resource/res_type/data/blend_state.h"

#include <string>
#include <vector>

namespace Pilot
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
        float           frame_position; // 0-1
        AnimationResult animation_result;
    };

    REFLECTION_TYPE(AnimationComponents)
    CLASS(AnimationComponents, Fields)
    {
        REFLECTION_BODY(AnimationComponents);

    public:
        std::string                        schemaFile;
        std::vector<AnimationComponentRes> components;
    };

} // namespace Pilot