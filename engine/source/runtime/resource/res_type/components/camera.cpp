#include "runtime/resource/res_type/components/camera.h"

#include "runtime/core/base/macro.h"

namespace Pilot
{
	CameraComponentRes::CameraComponentRes(const CameraComponentRes& res)
	{
		const std::string& camera_type_name = res.m_parameter.getTypeName();
		if (camera_type_name == "FirstPersonCameraParameter")
		{
			m_parameter = PILOT_REFLECTION_NEW(FirstPersonCameraParameter);
			*static_cast<FirstPersonCameraParameter*>(m_parameter) =
				*static_cast<FirstPersonCameraParameter*>(res.m_parameter.getPtr());
		}
		else if (camera_type_name == "ThirdPersonCameraParameter")
		{
			m_parameter = PILOT_REFLECTION_NEW(ThirdPersonCameraParameter);
			*static_cast<ThirdPersonCameraParameter*>(m_parameter) =
				*static_cast<ThirdPersonCameraParameter*>(res.m_parameter.getPtr());
		}
		else if (camera_type_name == "FreeCameraParameter")
		{
			m_parameter = PILOT_REFLECTION_NEW(FreeCameraParameter);
			*static_cast<FreeCameraParameter*>(m_parameter) =
				*static_cast<FreeCameraParameter*>(res.m_parameter.getPtr());
		}
		else
		{
			LOG_ERROR("invalid camera type");
		}
	}
}