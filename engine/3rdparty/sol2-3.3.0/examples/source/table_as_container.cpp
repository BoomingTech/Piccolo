#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


struct Vector {
	int x;
	int y;

	Vector() = default;

	Vector(int _x, int _y) : x { _x }, y { _y } {
	}
};

int main() {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<Vector>("Vector",
	     sol::constructors<Vector(), Vector(int, int)>(),
	     "x",
	     sol::property(&Vector::x, &Vector::x),
	     "y",
	     sol::property(&Vector::y, &Vector::y));

	lua.script(
	     "vectors = { Vector.new(3, 6), Vector.new(6, 3) }");
	auto vectors = lua["vectors"].get<std::vector<Vector>>();

	sol_c_assert(vectors[0].x == 3);
	sol_c_assert(vectors[0].y == 6);

	sol_c_assert(vectors[1].x == 6);
	sol_c_assert(vectors[1].y == 3);

	return 0;
}
