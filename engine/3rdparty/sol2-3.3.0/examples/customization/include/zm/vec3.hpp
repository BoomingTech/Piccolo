#pragma once

#ifndef ZM_VEC3_HPP
#define ZM_VEC3_HPP

#include <cstddef>

namespace zm {

	struct vec3 {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4201) // nonstandard extension used :
                                // nameless struct/union
#pragma warning(disable : 4324) // structure was padded due to
                                // alignment specifier
#endif
		union {
			float elements[3];
			struct {
				union {
					float x, r, s;
				};
				union {
					float y, g, t;
				};
				union {
					float z, b, p;
				};
			};
		};

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

		vec3() : x(0), y(0), z(0) {
		}

		vec3(float x_, float y_, float z_)
		: x(x_), y(y_), z(z_) {
		}

		constexpr float* data() {
			return elements;
		}

		constexpr const float* data() const {
			return this->elements;
		}

		constexpr float& operator[](std::size_t i) {
			return this->elements[i];
		}

		constexpr const float& operator[](
		     std::size_t i) const {
			return this->elements[i];
		}
	};
} // namespace zm

#endif // ZM_VEC3_HPP
