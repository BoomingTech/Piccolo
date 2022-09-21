#pragma once

#include "runtime/core/math/random.h"

#include <algorithm>
#include <cmath>
#include <limits>

#define CMP(x, y) (fabsf(x - y) < FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))

namespace Piccolo
{
    static const float Math_POS_INFINITY = std::numeric_limits<float>::infinity();
    static const float Math_NEG_INFINITY = -std::numeric_limits<float>::infinity();
    static const float Math_PI           = 3.14159265358979323846264338327950288f;
    static const float Math_ONE_OVER_PI  = 1.0f / Math_PI;
    static const float Math_TWO_PI       = 2.0f * Math_PI;
    static const float Math_HALF_PI      = 0.5f * Math_PI;
    static const float Math_fDeg2Rad     = Math_PI / 180.0f;
    static const float Math_fRad2Deg     = 180.0f / Math_PI;
    static const float Math_LOG2         = log(2.0f);
    static const float Math_EPSILON      = 1e-6f;

    static const float Float_EPSILON  = FLT_EPSILON;
    static const float Double_EPSILON = DBL_EPSILON;

    class Radian;
    class Angle;
    class Degree;

    class Vector2;
    class Vector3;
    class Vector4;
    class Matrix3x3;
    class Matrix4x4;
    class Quaternion;

    class Radian
    {
        float m_rad;

    public:
        explicit Radian(float r = 0) : m_rad(r) {}
        explicit Radian(const Degree& d);
        Radian& operator=(float f)
        {
            m_rad = f;
            return *this;
        }
        Radian& operator=(const Degree& d);

        float valueRadians() const { return m_rad; }
        float valueDegrees() const; // see bottom of this file
        float valueAngleUnits() const;

        void setValue(float f) { m_rad = f; }

        const Radian& operator+() const { return *this; }
        Radian        operator+(const Radian& r) const { return Radian(m_rad + r.m_rad); }
        Radian        operator+(const Degree& d) const;
        Radian&       operator+=(const Radian& r)
        {
            m_rad += r.m_rad;
            return *this;
        }
        Radian& operator+=(const Degree& d);
        Radian  operator-() const { return Radian(-m_rad); }
        Radian  operator-(const Radian& r) const { return Radian(m_rad - r.m_rad); }
        Radian  operator-(const Degree& d) const;
        Radian& operator-=(const Radian& r)
        {
            m_rad -= r.m_rad;
            return *this;
        }
        Radian& operator-=(const Degree& d);
        Radian  operator*(float f) const { return Radian(m_rad * f); }
        Radian  operator*(const Radian& f) const { return Radian(m_rad * f.m_rad); }
        Radian& operator*=(float f)
        {
            m_rad *= f;
            return *this;
        }
        Radian  operator/(float f) const { return Radian(m_rad / f); }
        Radian& operator/=(float f)
        {
            m_rad /= f;
            return *this;
        }

        bool operator<(const Radian& r) const { return m_rad < r.m_rad; }
        bool operator<=(const Radian& r) const { return m_rad <= r.m_rad; }
        bool operator==(const Radian& r) const { return m_rad == r.m_rad; }
        bool operator!=(const Radian& r) const { return m_rad != r.m_rad; }
        bool operator>=(const Radian& r) const { return m_rad >= r.m_rad; }
        bool operator>(const Radian& r) const { return m_rad > r.m_rad; }
    };

    /** Wrapper class which indicates a given angle value is in Degrees.
    @remarks
        Degree values are interchangeable with Radian values, and conversions
        will be done automatically between them.
    */
    class Degree
    {
        float m_deg; // if you get an error here - make sure to define/typedef 'float' first

    public:
        explicit Degree(float d = 0) : m_deg(d) {}
        explicit Degree(const Radian& r) : m_deg(r.valueDegrees()) {}
        Degree& operator=(float f)
        {
            m_deg = f;
            return *this;
        }
        Degree& operator=(const Degree& d) = default;
        Degree& operator=(const Radian& r)
        {
            m_deg = r.valueDegrees();
            return *this;
        }

        float valueDegrees() const { return m_deg; }
        float valueRadians() const; // see bottom of this file
        float valueAngleUnits() const;

        const Degree& operator+() const { return *this; }
        Degree        operator+(const Degree& d) const { return Degree(m_deg + d.m_deg); }
        Degree        operator+(const Radian& r) const { return Degree(m_deg + r.valueDegrees()); }
        Degree&       operator+=(const Degree& d)
        {
            m_deg += d.m_deg;
            return *this;
        }
        Degree& operator+=(const Radian& r)
        {
            m_deg += r.valueDegrees();
            return *this;
        }
        Degree  operator-() const { return Degree(-m_deg); }
        Degree  operator-(const Degree& d) const { return Degree(m_deg - d.m_deg); }
        Degree  operator-(const Radian& r) const { return Degree(m_deg - r.valueDegrees()); }
        Degree& operator-=(const Degree& d)
        {
            m_deg -= d.m_deg;
            return *this;
        }
        Degree& operator-=(const Radian& r)
        {
            m_deg -= r.valueDegrees();
            return *this;
        }
        Degree  operator*(float f) const { return Degree(m_deg * f); }
        Degree  operator*(const Degree& f) const { return Degree(m_deg * f.m_deg); }
        Degree& operator*=(float f)
        {
            m_deg *= f;
            return *this;
        }
        Degree  operator/(float f) const { return Degree(m_deg / f); }
        Degree& operator/=(float f)
        {
            m_deg /= f;
            return *this;
        }

        bool operator<(const Degree& d) const { return m_deg < d.m_deg; }
        bool operator<=(const Degree& d) const { return m_deg <= d.m_deg; }
        bool operator==(const Degree& d) const { return m_deg == d.m_deg; }
        bool operator!=(const Degree& d) const { return m_deg != d.m_deg; }
        bool operator>=(const Degree& d) const { return m_deg >= d.m_deg; }
        bool operator>(const Degree& d) const { return m_deg > d.m_deg; }
    };

    /** Wrapper class which identifies a value as the currently default angle
        type, as defined by Math::setAngleUnit.
    @remarks
        Angle values will be automatically converted between radians and degrees,
        as appropriate.
    */
    class Angle
    {
        float m_angle;

    public:
        explicit Angle(float angle) : m_angle(angle) {}
        Angle() { m_angle = 0; }

        explicit operator Radian() const;
        explicit operator Degree() const;
    };

    class Math
    {
    private:
        enum class AngleUnit
        {
            AU_DEGREE,
            AU_RADIAN
        };

        // angle units used by the api
        static AngleUnit k_AngleUnit;

    public:
        Math();

        static float abs(float value) { return std::fabs(value); }
        static bool  isNan(float f) { return std::isnan(f); }
        static float sqr(float value) { return value * value; }
        static float sqrt(float fValue) { return std::sqrt(fValue); }
        static float invSqrt(float value) { return 1.f / sqrt(value); }
        static bool  realEqual(float a, float b, float tolerance = std::numeric_limits<float>::epsilon());
        static float clamp(float v, float min, float max) { return std::clamp(v, min, max); }
        static float getMaxElement(float x, float y, float z) { return std::max({x, y, z}); }

        static float degreesToRadians(float degrees);
        static float radiansToDegrees(float radians);
        static float angleUnitsToRadians(float units);
        static float radiansToAngleUnits(float radians);
        static float angleUnitsToDegrees(float units);
        static float degreesToAngleUnits(float degrees);

        static float  sin(const Radian& rad) { return std::sin(rad.valueRadians()); }
        static float  sin(float value) { return std::sin(value); }
        static float  cos(const Radian& rad) { return std::cos(rad.valueRadians()); }
        static float  cos(float value) { return std::cos(value); }
        static float  tan(const Radian& rad) { return std::tan(rad.valueRadians()); }
        static float  tan(float value) { return std::tan(value); }
        static Radian acos(float value);
        static Radian asin(float value);
        static Radian atan(float value) { return Radian(std::atan(value)); }
        static Radian atan2(float y_v, float x_v) { return Radian(std::atan2(y_v, x_v)); }

        template<class T>
        static constexpr T max(const T A, const T B)
        {
            return std::max(A, B);
        }

        template<class T>
        static constexpr T min(const T A, const T B)
        {
            return std::min(A, B);
        }

        template<class T>
        static constexpr T max3(const T A, const T B, const T C)
        {
            return std::max({A, B, C});
        }

        template<class T>
        static constexpr T min3(const T A, const T B, const T C)
        {
            return std::min({A, B, C});
        }

        static Matrix4x4
        makeViewMatrix(const Vector3& position, const Quaternion& orientation, const Matrix4x4* reflect_matrix = nullptr);

        static Matrix4x4
        makeLookAtMatrix(const Vector3& eye_position, const Vector3& target_position, const Vector3& up_dir);

        static Matrix4x4 makePerspectiveMatrix(Radian fovy, float aspect, float znear, float zfar);

        static Matrix4x4
        makeOrthographicProjectionMatrix(float left, float right, float bottom, float top, float znear, float zfar);
        
        static Matrix4x4
        makeOrthographicProjectionMatrix01(float left, float right, float bottom, float top, float znear, float zfar);
    };

    // these functions could not be defined within the class definition of class
    // Radian because they required class Degree to be defined
    inline Radian::Radian(const Degree& d) : m_rad(d.valueRadians()) {}
    inline Radian& Radian::operator=(const Degree& d)
    {
        m_rad = d.valueRadians();
        return *this;
    }
    inline Radian Radian::operator+(const Degree& d) const { return Radian(m_rad + d.valueRadians()); }
    inline Radian& Radian::operator+=(const Degree& d)
    {
        m_rad += d.valueRadians();
        return *this;
    }
    inline Radian Radian::operator-(const Degree& d) const { return Radian(m_rad - d.valueRadians()); }
    inline Radian& Radian::operator-=(const Degree& d)
    {
        m_rad -= d.valueRadians();
        return *this;
    }

    inline float Radian::valueDegrees() const { return Math::radiansToDegrees(m_rad); }

    inline float Radian::valueAngleUnits() const { return Math::radiansToAngleUnits(m_rad); }

    inline float Degree::valueRadians() const { return Math::degreesToRadians(m_deg); }

    inline float Degree::valueAngleUnits() const { return Math::degreesToAngleUnits(m_deg); }

    inline Angle::operator Radian() const { return Radian(Math::angleUnitsToRadians(m_angle)); }

    inline Angle::operator Degree() const { return Degree(Math::angleUnitsToDegrees(m_angle)); }

    inline Radian operator*(float a, const Radian& b) { return Radian(a * b.valueRadians()); }

    inline Radian operator/(float a, const Radian& b) { return Radian(a / b.valueRadians()); }

    inline Degree operator*(float a, const Degree& b) { return Degree(a * b.valueDegrees()); }

    inline Degree operator/(float a, const Degree& b) { return Degree(a / b.valueDegrees()); }
} // namespace Piccolo
