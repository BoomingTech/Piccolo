#pragma once

#include "runtime/core/math/math_headers.h"

#include <mutex>

namespace Piccolo
{
    enum class RenderCameraType : int
    {
        Editor,
        Motor
    };

    class RenderCamera
    {
    public:
        RenderCameraType m_current_camera_type {RenderCameraType::Editor};

        static const Vector3 X, Y, Z;

        Vector3    m_position {0.0f, 0.0f, 0.0f};
        Quaternion m_rotation {Quaternion::IDENTITY};
        Quaternion m_invRotation {Quaternion::IDENTITY};
        float      m_znear {1000.0f};
        float      m_zfar {0.1f};
        Vector3    m_up_axis {Z};

        static constexpr float MIN_FOV {10.0f};
        static constexpr float MAX_FOV {89.0f};
        static constexpr int   MAIN_VIEW_MATRIX_INDEX {0};

        std::vector<Matrix4x4> m_view_matrices {Matrix4x4::IDENTITY};

        void setCurrentCameraType(RenderCameraType type);
        void setMainViewMatrix(const Matrix4x4& view_matrix, RenderCameraType type = RenderCameraType::Editor);

        void move(Vector3 delta);
        void rotate(Vector2 delta);
        void zoom(float offset);
        void lookAt(const Vector3& position, const Vector3& target, const Vector3& up);

        void setAspect(float aspect);
        void setFOVx(float fovx) { m_fovx = fovx; }

        Vector3    position() const { return m_position; }
        Quaternion rotation() const { return m_rotation; }

        Vector3   forward() const { return (m_invRotation * Y); }
        Vector3   up() const { return (m_invRotation * Z); }
        Vector3   right() const { return (m_invRotation * X); }
        Vector2   getFOV() const { return {m_fovx, m_fovy}; }
        Matrix4x4 getViewMatrix();
        Matrix4x4 getPersProjMatrix() const;
        Matrix4x4 getLookAtMatrix() const { return Math::makeLookAtMatrix(position(), position() + forward(), up()); }
        float     getFovYDeprecated() const { return m_fovy; }

    protected:
        float m_aspect {0.f};
        float m_fovx {Degree(89.f).valueDegrees()};
        float m_fovy {0.f};

        std::mutex m_view_matrix_mutex;
    };

    inline const Vector3 RenderCamera::X = {1.0f, 0.0f, 0.0f};
    inline const Vector3 RenderCamera::Y = {0.0f, 1.0f, 0.0f};
    inline const Vector3 RenderCamera::Z = {0.0f, 0.0f, 1.0f};

} // namespace Piccolo
