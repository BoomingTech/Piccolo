#pragma once
#include "runtime/core/meta/reflection/reflection.h"
#include <string>
#include <vector>
namespace Piccolo
{

    REFLECTION_TYPE(BoneBlendMask)
    CLASS(BoneBlendMask, Fields)
    {
        REFLECTION_BODY(BoneBlendMask);

    public:
        std::string      m_skeleton_file_path;
        std::vector<int> m_enabled;
    };

} // namespace Piccolo