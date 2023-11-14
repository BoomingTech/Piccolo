#include "anim_instance.h"
#include "function/input/input_system.h"
#include "function/render/render_type.h"

using namespace Piccolo;

namespace Piccolo
{
    Transform CAnimInstanceBase::GetRootMotion()
    {
	    return {Vector3::ZERO, Quaternion::IDENTITY};
    }

	bool CAnimInstanceBase::HasRootMotion()
	{
		return true;
	}
} // namespace Piccolo
