#include "runtime/function/physics/ray.h"

namespace Pilot
{
    Ray::Ray(Vector3 start_point, Vector3 direction)
    {
        m_start_point = start_point;
        m_direction   = direction;
    }

    Ray::~Ray() {}

    Vector3 Ray::getStartPoint() const { return m_start_point; }

    Vector3 Ray::getDirection() const { return m_direction; }

} // namespace Pilot
