#pragma once

#include "runtime/core/math/vector3.h"

#include "runtime/resource/res_type/components/camera.h"

#include "runtime/function/framework/component/component.h"

namespace Pilot
{
    enum class CameraMode : unsigned char
    {
        third_person,
        first_person,
        free,
        invalid
    };

    REFLECTION_TYPE(CameraComponent)
    CLASS(CameraComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(CameraComponent)
    protected:
        CameraComponentRes m_camera_param;

        CameraMode m_camera_mode {CameraMode::invalid};
        META(Enable)
        Vector3 m_foward {Vector3::NEGATIVE_UNIT_Y};
        META(Enable)
        Vector3 m_up {Vector3::UNIT_Z};
        META(Enable)
        Vector3 m_left {Vector3::UNIT_X};

    public:
        CameraComponent() {}
        CameraComponent(const CameraComponentRes& camera_param, GObject* parent_object);

        void tick(float delta_time) override;
        void destroy() override {}

    private:
        void tickFirstPersonCamera(float delta_time);
        void tickThirdPersonCamera(float delta_time);
        void tickFreeCamera();
    };
} // namespace Pilot
