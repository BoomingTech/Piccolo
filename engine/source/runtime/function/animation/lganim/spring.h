#pragma once

#include "common.h"
#include "runtime/core/math/vector3.h"
#include "runtime/core/math/quaternion.h"

//--------------------------------------
namespace Piccolo
{
	static inline float damper_exact(float x, float g, float halflife, float dt, float eps = 1e-5f)
	{
		return lerpf(x, g, 1.0f - fast_negexpf((LN2f * dt) / (halflife + eps)));
	}

	static inline Vector3 damper_exact(Vector3 x, Vector3 g, float halflife, float dt, float eps = 1e-5f)
	{
		return Vector3::lerp(x, g, 1.0f - fast_negexpf((LN2f * dt) / (halflife + eps)));
	}

	static inline Quaternion damper_exact(Quaternion x, Quaternion g, float halflife, float dt, float eps = 1e-5f)
	{
		return Quaternion::sLerp(1.0f - fast_negexpf((LN2f * dt) / (halflife + eps)), x, g, true);
	}

	static inline float damp_adjustment_exact(float g, float halflife, float dt, float eps = 1e-5f)
	{
		return g * (1.0f - fast_negexpf((LN2f * dt) / (halflife + eps)));
	}

	static inline Vector3 damp_adjustment_exact(Vector3 g, float halflife, float dt, float eps = 1e-5f)
	{
		return g * (1.0f - fast_negexpf((LN2f * dt) / (halflife + eps)));
	}

	static inline Quaternion damp_adjustment_exact(Quaternion g, float halflife, float dt, float eps = 1e-5f)
	{
		return Quaternion::sLerp(1.0f - fast_negexpf((LN2f * dt) / (halflife + eps)), Quaternion::IDENTITY, g);
	}

	//--------------------------------------

	static inline float halflife_to_damping(float halflife, float eps = 1e-5f) { return (4.0f * LN2f) / (halflife + eps); }

	static inline float damping_to_halflife(float damping, float eps = 1e-5f) { return (4.0f * LN2f) / (damping + eps); }

	static inline float frequency_to_stiffness(float frequency) { return squaref(2.0f * PIf * frequency); }

	static inline float stiffness_to_frequency(float stiffness) { return sqrtf(stiffness) / (2.0f * PIf); }

	//--------------------------------------

	static inline void
	simple_spring_damper_exact(float& x, float& v, const float x_goal, const float halflife, const float dt)
	{
		float y = halflife_to_damping(halflife) / 2.0f;
		float j0 = x - x_goal;
		float j1 = v + j0 * y;
		float eydt = fast_negexpf(y * dt);

		x = eydt * (j0 + j1 * dt) + x_goal;
		v = eydt * (v - j1 * y * dt);
	}

	static inline void simple_spring_damper_exact(Vector3& x, Vector3& v, const Vector3 x_goal, const float halflife, const float dt)
	{
		float y = halflife_to_damping(halflife) / 2.0f;
		Vector3 j0 = x - x_goal;
		Vector3 j1 = v + j0 * y;
		float eydt = fast_negexpf(y * dt);

		x = eydt * (j0 + j1 * dt) + x_goal;
		v = eydt * (v - j1 * y * dt);
	}

	static inline void simple_spring_damper_exact(Quaternion& x, Vector3& v, const Quaternion x_goal, const float halflife, const float dt)
	{
		float y = halflife_to_damping(halflife) / 2.0f;

		Quaternion temp = x * x_goal.inverse();
		temp.abs();
		const Vector3 j0 = temp.toAngleAxis();
		const Vector3 j1 = v + j0 * y;

		const float eydt = fast_negexpf(y * dt);

		x = Quaternion(eydt * (j0 + j1 * dt)) * x_goal;
		v = eydt * (v - j1 * y * dt);
	}

	//--------------------------------------

	static inline void decay_spring_damper_exact(float& x, float& v, const float halflife, const float dt)
	{
		float y = halflife_to_damping(halflife) / 2.0f;
		float j1 = v + x * y;
		float eydt = fast_negexpf(y * dt);

		x = eydt * (x + j1 * dt);
		v = eydt * (v - j1 * y * dt);
	}

	static inline void decay_spring_damper_exact(Vector3& x, Vector3& v, const float halflife, const float dt)
	{
		float y = halflife_to_damping(halflife) / 2.0f;
		Vector3 j1 = v + x * y;
		float eydt = fast_negexpf(y * dt);

		x = eydt * (x + j1 * dt);
		v = eydt * (v - j1 * y * dt);
	}

	static inline void decay_spring_damper_exact(Quaternion& x, Vector3& v, const float halflife, const float dt)
	{
		float y = halflife_to_damping(halflife) / 2.0f;

		Vector3 j0 = x.toAngleAxis();
		Vector3 j1 = v + j0 * y;

		float eydt = fast_negexpf(y * dt);

		x = Quaternion(eydt * (j0 + j1 * dt));
		v = eydt * (v - j1 * y * dt);
	}

	//--------------------------------------

	static inline void inertialize_transition(Vector3& off_x, Vector3& off_v, const Vector3 src_x, const Vector3 src_v, const Vector3 dst_x, const Vector3 dst_v)
	{
		off_x = (src_x + off_x) - dst_x;
		off_v = (src_v + off_v) - dst_v;
	}

	static inline void inertialize_update(Vector3& out_x,
	                                      Vector3& out_v,
	                                      Vector3& off_x,
	                                      Vector3& off_v,
	                                      const Vector3 in_x,
	                                      const Vector3 in_v,
	                                      const float halflife,
	                                      const float dt)
	{
		decay_spring_damper_exact(off_x, off_v, halflife, dt);
		out_x = in_x + off_x;
		out_v = in_v + off_v;
	}

	static inline void inertialize_transition(Quaternion& off_x, Vector3& off_v, const Quaternion src_x, const Vector3 src_v, const Quaternion dst_x, const Vector3 dst_v)
	{
        off_x = off_x * src_x* dst_x.inverse();
        off_x.abs();
		off_v = (off_v + src_v) - dst_v;
	}

	static inline void inertialize_update(Quaternion& out_x,
	                                      Vector3& out_v,
	                                      Quaternion& off_x,
	                                      Vector3& off_v,
	                                      const Quaternion in_x,
	                                      const Vector3 in_v,
	                                      const float halflife,
	                                      const float dt)
	{
		decay_spring_damper_exact(off_x, off_v, halflife, dt);
		out_x = off_x* in_x;
		out_v = off_v + in_v;
	}
} // namespace Piccolo
