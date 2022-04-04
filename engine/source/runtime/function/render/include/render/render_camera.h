#pragma once

#include "runtime/core/math/math_headers.h"

namespace Pilot
{
    enum class PCurrentCameraType : int
    {
        Editor,
        Motor
    };

    class PCamera
    {
        float m_aspect;
        float m_fovx {Degree(89.f).valueDegrees()};
        float m_fovy;

    public:
        Vector3    m_position {0.0f, 0.0f, 0.0f};
        Quaternion m_rotation {Quaternion::IDENTITY};
        Quaternion m_invRotation {Quaternion::IDENTITY};
        float      m_znear {1000.0f};
        float      m_zfar {0.1f};

        float m_exposure {1.0f};

        static const Vector3 X, Y, Z;

        static constexpr float MIN_FOV  = 10.0f;
        static constexpr float MAX_FOV  = 89.0f;
        Vector3                m_upAxis = Z;

        std::mutex             m_view_matrix_mutex;
        static const int       MAIN_VIEW_MATRIX_INDEX = 0;
        std::vector<Matrix4x4> m_view_matrices        = {Matrix4x4::IDENTITY};

        PCurrentCameraType m_current_camera_type {PCurrentCameraType::Editor};
        void               setCurrentCameraType(PCurrentCameraType type)
        {
            std::lock_guard<std::mutex> lock_guard(m_view_matrix_mutex);
            m_current_camera_type = type;
        }
        void setMainViewMatrix(const Matrix4x4& view_matrix, PCurrentCameraType type = PCurrentCameraType::Editor)
        {
            std::lock_guard<std::mutex> lock_guard(m_view_matrix_mutex);
            m_current_camera_type                   = type;
            m_view_matrices[MAIN_VIEW_MATRIX_INDEX] = view_matrix;

            Vector3 s  = Vector3(view_matrix[0][0], view_matrix[0][1], view_matrix[0][2]);
            Vector3 u  = Vector3(view_matrix[1][0], view_matrix[1][1], view_matrix[1][2]);
            Vector3 f  = Vector3(-view_matrix[2][0], -view_matrix[2][1], -view_matrix[2][2]);
            m_position = s * (-view_matrix[0][3]) + u * (-view_matrix[1][3]) + f * view_matrix[2][3];
        }

        void move(Vector3 delta) // coordinates are in world-space (use forward/up/right to
                                 // move
                                 // relative to the camera)
        {
            m_position += delta;
        };
        void rotate(Vector2 delta) // rotation around x, y axis
        {
            delta = Vector2(Radian(Degree(delta.x)).valueRadians(), Radian(Degree(delta.y)).valueRadians());

            // limit pitch
            float dot = m_upAxis.dotProduct(forward());
            if ((dot < -0.99f && delta.x > 0.0f) || // angle nearing 180 degrees
                (dot > 0.99f && delta.x < 0.0f))    // angle nearing 0 degrees
                delta.x = 0.0f;

            // pitch is relative to current sideways rotation
            // yaw happens independently
            // this prevents roll
            Quaternion pitch, yaw;
            pitch.fromAngleAxis(Radian(delta.x), X);
            yaw.fromAngleAxis(Radian(delta.y), Z);

            m_rotation = pitch * m_rotation * yaw;

            m_invRotation = m_rotation.conjugate();
        }
        void zoom(float offset) // > 0 = zoom in (decrease FOV by <offset> angles)
        {
            m_fovx = Math::clamp(m_fovx - offset, MIN_FOV, MAX_FOV);
        }

        void lookAt(const Vector3& position, const Vector3& target, const Vector3& up)
        {
            {
                m_position = position;

                // model rotation
                // maps vectors to camera space (x, y, z)
                Vector3 forward = (target - position).normalisedCopy();
                m_rotation      = forward.getRotationTo(Y);

                // correct the up vector
                // the cross product of non-orthogonal vectors is not normalized
                Vector3 right  = forward.crossProduct(up.normalisedCopy()).normalisedCopy();
                Vector3 orthUp = right.crossProduct(forward);

                Quaternion upRotation = (m_rotation * orthUp).getRotationTo(Z);

                m_rotation = Quaternion(upRotation) * m_rotation;

                // inverse of the model rotation
                // maps camera space vectors to model vectors
                m_invRotation = m_rotation.conjugate();
            }
        }

        Vector3    position() const { return m_position; }
        Quaternion rotation() const { return m_rotation; }

        // view matrix
        Matrix4x4 getViewMatrix()
        {
            std::lock_guard<std::mutex> lock_guard(m_view_matrix_mutex);
            auto                        view_matrix = Matrix4x4::IDENTITY;
            switch (m_current_camera_type)
            {
                case Pilot::PCurrentCameraType::Editor:
                    view_matrix = Math::makeLookAtMatrix(position(), position() + forward(), up());
                    break;
                case Pilot::PCurrentCameraType::Motor:
                    view_matrix = m_view_matrices[MAIN_VIEW_MATRIX_INDEX];
                    break;
                default:
                    break;
            }
            return view_matrix;
        }

        Matrix4x4 getPersProjMatrix() const
        {
            Matrix4x4 fix_mat(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
            Matrix4x4 proj_mat =
                fix_mat * Math::makePerspectiveMatrix(Radian(Degree(m_fovy)), m_aspect, m_znear, m_zfar);

            return proj_mat;
        }

        Matrix4x4 getLookAtMatrix() const { return Math::makeLookAtMatrix(position(), position() + forward(), up()); }

        float getFovYDeprecated() const { return m_fovy; }

        void setAspect(float aspect)
        {
            m_aspect = aspect;

            // 1 / tan(fovy * 0.5) / aspect = 1 / tan(fovx * 0.5)
            // 1 / tan(fovy * 0.5) = aspect / tan(fovx * 0.5)
            // tan(fovy * 0.5) = tan(fovx * 0.5) / aspect

            m_fovy = Radian(Math::atan(Math::tan(Radian(Degree(m_fovx) * 0.5f)) / m_aspect) * 2.0f).valueDegrees();
        }

        void setFOVx(float fovx) { m_fovx = fovx; }

        const Vector2 getFOV() const { return Vector2(m_fovx, m_fovy); }

        // camera vectors in world-space coordinates
        Vector3 forward() const { return (m_invRotation * Y); }
        Vector3 up() const { return (m_invRotation * Z); }
        Vector3 right() const { return (m_invRotation * X); }
    };

    inline const Vector3 PCamera::X = {1.0f, 0.0f, 0.0f};
    inline const Vector3 PCamera::Y = {0.0f, 1.0f, 0.0f};
    inline const Vector3 PCamera::Z = {0.0f, 0.0f, 1.0f};

} // namespace Pilot