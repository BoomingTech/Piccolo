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
        int        m_index;
        Matrix4x4_ m_transform;
    };

    REFLECTION_TYPE(AnimationResult)
    CLASS(AnimationResult, Fields)
    {
        REFLECTION_BODY(AnimationResult);

    public:
        std::vector<AnimationResultElement> m_node;
    };

    REFLECTION_TYPE(AnimationComponentRes)
    CLASS(AnimationComponentRes, WhiteListFields)
    {
        REFLECTION_BODY(AnimationComponentRes);

    public:

        META(Enable) 
        std::string m_skeleton_file_path;
        META(Enable) 
        std::vector<Reflection::ReflectionPtr<ClipBase>> m_clips;
        // animation to skeleton map
    };

} // namespace Pilot