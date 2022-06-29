#pragma once
#include "runtime/core/math/quaternion.h"
#include "runtime/core/meta/reflection/reflection.h"

namespace Piccolo
{
    REFLECTION_TYPE(CameraParameter)
    CLASS(CameraParameter, Fields)
    {
        REFLECTION_BODY(CameraParameter);

    public:
        float m_fov {50.f};

        virtual ~CameraParameter() {}
    };

    REFLECTION_TYPE(FirstPersonCameraParameter)
    CLASS(FirstPersonCameraParameter : public CameraParameter, Fields)
    {
        REFLECTION_BODY(FirstPersonCameraParameter);

    public:
        float m_vertical_offset {0.6f};
    };

    REFLECTION_TYPE(ThirdPersonCameraParameter)
    CLASS(ThirdPersonCameraParameter : public CameraParameter, WhiteListFields)
    {
        REFLECTION_BODY(ThirdPersonCameraParameter);

    public:
        META(Enable)
        float m_horizontal_offset {3.f};
        META(Enable)
        float      m_vertical_offset {2.5f};
        Quaternion m_cursor_pitch;
        Quaternion m_cursor_yaw;
    };

    REFLECTION_TYPE(FreeCameraParameter)
    CLASS(FreeCameraParameter : public CameraParameter, Fields)
    {
        REFLECTION_BODY(FreeCameraParameter);

    public:
        float m_speed {1.f};
    };

    REFLECTION_TYPE(CameraComponentRes)
    CLASS(CameraComponentRes, Fields)
    {
        REFLECTION_BODY(CameraComponentRes);

    public:
        Reflection::ReflectionPtr<CameraParameter> m_parameter;

        CameraComponentRes() = default;
        CameraComponentRes(const CameraComponentRes& res);

        ~CameraComponentRes();
    };
} // namespace Piccolo