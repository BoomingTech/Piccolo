#pragma once

#include "runtime/core/math/vector3.h"

#include <limits>

namespace Pilot
{
    class BoundingBox
    {
    public:
        Vector3 m_min {std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max()};
        Vector3 m_max {std::numeric_limits<float>::min(),
                       std::numeric_limits<float>::min(),
                       std::numeric_limits<float>::min()};

        void merge(const Vector3& point)
        {
            m_min.makeFloor(point);
            m_max.makeCeil(point);
        }
    };
} // namespace Pilot
