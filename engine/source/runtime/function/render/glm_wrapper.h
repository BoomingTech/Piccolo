#pragma once

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS 1
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#endif

#include "runtime/core/math/math_headers.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Pilot
{
    class GLMUtil
    {
    public:
        static Vector3   toVec3(glm::vec3 v) { return Vector3(v.x, v.y, v.z); }
        static glm::vec3 fromVec3(Vector3 v) { return glm::vec3(v.x, v.y, v.z); }

        static Vector4   toVec4(glm::vec4 v) { return Vector4(v.x, v.y, v.z, v.w); }
        static glm::vec4 fromVec4(Vector4 v) { return glm::vec4(v.x, v.y, v.z, v.w); }

        static Quaternion toQuat(glm::quat q) { return Quaternion(q.w, q.x, q.y, q.z); }
        static glm::quat  fromQuat(Quaternion q) { return glm::quat(q.w, q.x, q.y, q.z); }

        static Matrix4x4   toMat4x4(const glm::mat4x4& m);
        static glm::mat4x4 fromMat4x4(const Matrix4x4& m);
    };

    inline Matrix4x4 GLMUtil::toMat4x4(const glm::mat4x4& m)
    {
        return Matrix4x4 {m[0][0],
                          m[1][0],
                          m[2][0],
                          m[3][0],
                          m[0][1],
                          m[1][1],
                          m[2][1],
                          m[3][1],
                          m[0][2],
                          m[1][2],
                          m[2][2],
                          m[3][2],
                          m[0][3],
                          m[1][3],
                          m[2][3],
                          m[3][3]};
    }

    inline glm::mat4x4 GLMUtil::fromMat4x4(const Matrix4x4& m)
    {
        return glm::mat4x4 {m[0][0],
                            m[1][0],
                            m[2][0],
                            m[3][0],
                            m[0][1],
                            m[1][1],
                            m[2][1],
                            m[3][1],
                            m[0][2],
                            m[1][2],
                            m[2][2],
                            m[3][2],
                            m[0][3],
                            m[1][3],
                            m[2][3],
                            m[3][3]};
    }
} // namespace Pilot