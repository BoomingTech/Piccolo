// sol2

// The MIT License (MIT)

// Copyright (c) 2013-2022 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "sol_test.hpp"

#include "common_classes.hpp"

#include <catch2/catch_all.hpp>

inline namespace sol2_test_usertypes_constructors {

	struct matrix_xf {
		float a, b;

		static matrix_xf from_lua_table(sol::table t) {
			matrix_xf m;
			m.a = t[1][1];
			m.b = t[1][2];
			return m;
		}
	};

	struct matrix_xi {
		int a, b;

		static matrix_xi from_lua_table(sol::table t) {
			matrix_xi m;
			m.a = t[1][1];
			m.b = t[1][2];
			return m;
		}
	};

	struct constructor_cheat {

		int val;

		constexpr constructor_cheat() noexcept;
		constexpr constructor_cheat(int val_) noexcept : val(val_) {
		}
	};

	namespace Helpers {
		class vector2 {
		public:
			vector2() : x(0), y(0) {
			}
			vector2(float x, float y) : x(x), y(y) {
			}
			vector2(const vector2& _vector2) : x(_vector2.x), y(_vector2.y) {
			}

			float x, y;

			vector2& operator+=(const vector2& _vector2);

			vector2& operator-=(const vector2& _vector2);

			vector2& operator*=(const float scalar);

			vector2& operator*=(const vector2& _vector2);

			/*std::shared_ptr<vector2> operator^=(const std::shared_ptr<vector2> _vector2)
			{
			     this->x *= _vector2->y;
			     this->y *= _vector2->x;

			     return std::make_shared<vector2>(*this);
			}*/

			friend float operator*(const vector2& _vector, const vector2& _vector2) {
				return _vector.x * _vector2.x + _vector.y * _vector2.y;
			}

			friend float operator^(const vector2& _vector, const vector2& _vector2) {
				return _vector.x * _vector2.y + _vector2.x * _vector.y;
			}

			vector2& operator=(const vector2& _vector2) noexcept;
		};
	} // namespace Helpers

	namespace Helpers {
		class vector3 {
		public:
			vector3() : x(0), y(0), z(0) {
			}
			vector3(float x, float y, float z) : x(x), y(y), z(z) {
			}
			vector3(const vector2& _vector2, float z) : x(_vector2.x), y(_vector2.y), z(z) {
			}
			float x, y, z;

			float GetX() {
				return x;
			}

			void SetX(float new_x) {
				x = new_x;
			}

			vector3& operator+=(const vector3& _vector2);

			vector3& operator-=(const vector3& _vector2);

			vector3& operator*=(const float scalar);

			vector3& operator*=(const vector3& _vector2);

			friend float operator*(const vector3& _vector, const vector3& _vector2) {
				return _vector.x * _vector2.x + _vector.y * _vector2.y + _vector.z * _vector2.z;
			}

			friend float operator^(const vector3& _vector, const vector3& _vector2) {
				return _vector.x * _vector2.y + _vector2.x * _vector.y;
			}

			vector3& operator=(const vector3& _vector2) noexcept;
		};
	} // namespace Helpers
} // namespace sol2_test_usertypes_constructors

TEST_CASE("usertype/call_constructor", "make sure lua types can be constructed with function call constructors") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<thing>("thing", "v", &thing::v, sol::call_constructor, sol::constructors<sol::types<>, sol::types<int>>());

	lua.safe_script(R"(
t = thing(256)
)");

	thing& y = lua["t"];
	INFO(y.v);
	REQUIRE(y.v == 256);
}

TEST_CASE("usertype/call_constructor factories", "make sure tables can be passed to factory-based call constructors") {
	sol::state lua;
	lua.open_libraries();

	lua.new_usertype<matrix_xf>("mat", sol::call_constructor, sol::factories(&matrix_xf::from_lua_table));

	lua.safe_script("m = mat{ {1.1, 2.2} }");

	lua.new_usertype<matrix_xi>("mati", sol::call_constructor, sol::factories(&matrix_xi::from_lua_table));

	lua.safe_script("mi = mati{ {1, 2} }");

	matrix_xf& m = lua["m"];
	REQUIRE(m.a == 1.1f);
	REQUIRE(m.b == 2.2f);
	matrix_xi& mi = lua["mi"];
	REQUIRE(mi.a == 1);
	REQUIRE(mi.b == 2);
}

TEST_CASE("usertype/call_constructor metatable check", "prevent metatable regression") {
	class class01 {
	public:
		int x = 57;
		class01() {
		}
	};

	class class02 {
	public:
		int x = 50;
		class02() {
		}
		class02(const class01& other) : x(other.x) {
		}
	};

	sol::state lua;

	lua.new_usertype<class01>("class01", sol::call_constructor, sol::constructors<sol::types<>, sol::types<const class01&>>());

	lua.new_usertype<class02>("class02", sol::call_constructor, sol::constructors<sol::types<>, sol::types<const class02&>, sol::types<const class01&>>());

	REQUIRE_NOTHROW(lua.safe_script(R"(
x = class01()
y = class02(x)
)"));
	class02& y = lua["y"];
	REQUIRE(y.x == 57);
}

TEST_CASE("usertype/blank_constructor", "make sure lua types cannot be constructed with arguments if a blank / empty constructor is provided") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<thing>("thing", "v", &thing::v, sol::call_constructor, sol::constructors<>());

	auto result = lua.safe_script("t = thing(256)", sol::script_pass_on_error);
	REQUIRE_FALSE(result.valid());
}

TEST_CASE("usertype/no_constructor", "make sure lua types cannot be constructed if a blank / empty constructor is provided") {
	SECTION("order1") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		lua.new_usertype<thing>("thing", "v", &thing::v, sol::call_constructor, sol::no_constructor);
		auto result = lua.safe_script("t = thing()", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}

	SECTION("order2") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<thing>("thing", sol::call_constructor, sol::no_constructor, "v", &thing::v);
		auto result = lua.safe_script("t = thing.new()", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}

	SECTION("new no_constructor") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<thing>("thing", sol::meta_function::construct, sol::no_constructor);
		auto result = lua.safe_script("t = thing.new()", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}

	SECTION("call no_constructor") {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<thing>("thing", sol::call_constructor, sol::no_constructor);
		auto result = lua.safe_script("t = thing()", sol::script_pass_on_error);
		REQUIRE_FALSE(result.valid());
	}
}

TEST_CASE("usertype/constructor list", "Show that we can create classes from usertype and use them with multiple constructors") {

	sol::state lua;

	sol::constructors<sol::types<>, sol::types<int>, sol::types<int, int>> con;
	sol::usertype<crapola::fuser> lc = lua.new_usertype<crapola::fuser>("fuser", con, "add", &crapola::fuser::add, "add2", &crapola::fuser::add2);

	lua.safe_script(
	     "a = fuser.new(2)\n"
	     "u = a:add(1)\n"
	     "v = a:add2(1)\n"
	     "b = fuser:new()\n"
	     "w = b:add(1)\n"
	     "x = b:add2(1)\n"
	     "c = fuser.new(2, 3)\n"
	     "y = c:add(1)\n"
	     "z = c:add2(1)\n");
	sol::object a = lua.get<sol::object>("a");
	auto atype = a.get_type();
	REQUIRE((atype == sol::type::userdata));
	sol::object u = lua.get<sol::object>("u");
	sol::object v = lua.get<sol::object>("v");
	REQUIRE((u.as<int>() == 3));
	REQUIRE((v.as<int>() == 5));

	sol::object b = lua.get<sol::object>("b");
	auto btype = b.get_type();
	REQUIRE((btype == sol::type::userdata));
	sol::object w = lua.get<sol::object>("w");
	sol::object x = lua.get<sol::object>("x");
	REQUIRE((w.as<int>() == 1));
	REQUIRE((x.as<int>() == 3));

	sol::object c = lua.get<sol::object>("c");
	auto ctype = c.get_type();
	REQUIRE((ctype == sol::type::userdata));
	sol::object y = lua.get<sol::object>("y");
	sol::object z = lua.get<sol::object>("z");
	REQUIRE((y.as<int>() == 7));
	REQUIRE((z.as<int>() == 9));
}

TEST_CASE("usertype/no_constructor linking time", "make sure if no constructor is present, do not fix anything") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	auto ut = lua.new_usertype<constructor_cheat>("constructor_cheat", sol::no_constructor);
	ut["val"] = &constructor_cheat::val;

	lua["heck"] = constructor_cheat(1);

	sol::optional<sol::error> maybe_error = lua.safe_script("assert(heck.val == 1)", &sol::script_pass_on_error);
	REQUIRE_FALSE(maybe_error.has_value());
}

TEST_CASE("usertypes/constructors_match", "Ensure that objects are properly constructed even in the case of multi-match, stack-cleaning constructors") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);
	sol::table Lnamespace = lua["Helpers"].get_or_create<sol::table>();

	Lnamespace.new_usertype<Helpers::vector2>("vector2",
	     sol::constructors<Helpers::vector2(), Helpers::vector3(float, float), Helpers::vector2(const Helpers::vector2&)>(),
	     "x",
	     &Helpers::vector2::x,
	     "y",
	     &Helpers::vector2::y);

	Lnamespace.new_usertype<Helpers::vector3>("vector3",
	     sol::constructors<Helpers::vector3(), Helpers::vector3(float, float, float), Helpers::vector3(Helpers::vector2&, float)>(),
	     "x",
	     &Helpers::vector3::x,
	     "y",
	     &Helpers::vector3::y,
	     "z",
	     &Helpers::vector3::z);

	auto result = lua.safe_script(R"(local obj = Helpers.vector3.new(5,7,2)
		assert(obj.x == 5)
	)",
	     sol::script_pass_on_error);
	REQUIRE(result.valid());
}
