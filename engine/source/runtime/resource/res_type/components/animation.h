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
    CLASS(AnimationComponentRes, Fields)
    {
        REFLECTION_BODY(AnimationComponentRes);

    public:
        META(Enable)
        std::string m_skeleton_file_path;
        META(Enable) 
        std::vector<Reflection::ReflectionPtr<ClipBase>> m_clips;

        //BlendState  m_blend_state;
        // animation to skeleton map
        //float m_frame_position; // 0-1
        //AnimationResult m_animation_result;
    };

} // namespace Piccolo