#define SOL_ALL_SAFETIES_ON 1
#define SOL_ENABLE_INTEROP \
	1 // MUST be defined to use interop features
#include <sol/sol.hpp>

#include <luwra.hpp>

#include <iostream>

// luwra,
// another C++ wrapper library:
// https://github.com/vapourismo/luwra

struct ABC {
	ABC() : v_(0) {
	}
	ABC(int value) : v_(value) {
	}
	int value() const {
		return v_;
	}
	void setValue(int v) {
		v_ = v;
	}
	void overload1() {
		std::cout << "call overload1" << std::endl;
	}
	void overload2(int) {
		std::cout << "call overload2" << std::endl;
	}

private:
	int v_;
};

template <typename T, typename Handler>
inline bool sol_lua_interop_check(sol::types<T>, lua_State* L,
     int relindex, sol::type index_type, Handler&& handler,
     sol::stack::record& tracking) {
	// just marking unused parameters for no compiler warnings
	(void)index_type;
	(void)handler;
	// using 1 element
	tracking.use(1);
	int index = lua_absindex(L, relindex);
	if (lua_getmetatable(L, index) == 1) {
		luaL_getmetatable(
		     L, luwra::internal::UserTypeReg<T>::name.c_str());
		bool is_correct_type = lua_rawequal(L, -2, -1) == 1;
		lua_pop(L, 2);
		return is_correct_type;
	}
	return false;
}

template <typename T>
inline std::pair<bool, T*> sol_lua_interop_get(sol::types<T> t,
     lua_State* L, int relindex, void* unadjusted_pointer,
     sol::stack::record& tracking) {
	// you may not need to specialize this method every time:
	// some libraries are compatible with sol2's layout
	int index = lua_absindex(L, relindex);
	if (!sol_lua_interop_check(t,
	         L,
	         index,
	         sol::type::userdata,
	         sol::no_panic,
	         tracking)) {
		return { false, nullptr };
	}
	return { true, static_cast<T*>(unadjusted_pointer) };
}

void register_sol_stuff(lua_State* L) {
	// grab raw state and put into state_view
	// state_view is cheap to construct
	sol::state_view lua(L);
	// bind and set up your things: everything is entirely
	// self-contained
	lua["f"] = sol::overload(
	     [](ABC& from_luwra) {
		     std::cout << "calling 1-argument version with "
		                  "luwra-created ABC { "
		               << from_luwra.value() << " }"
		               << std::endl;
		     sol_c_assert(from_luwra.value() == 24);
	     },
	     [](ABC& from_luwra, int second_arg) {
		     std::cout << "calling 2-argument version with "
		                  "luwra-created ABC { "
		               << from_luwra.value()
		               << " } and integer argument of "
		               << second_arg << std::endl;
		     sol_c_assert(from_luwra.value() == 24);
		     sol_c_assert(second_arg == 5);
	     });
}

void check_with_sol(lua_State* L) {
	sol::state_view lua(L);
	ABC& obj = lua["obj"];
	(void)obj;
	sol_c_assert(obj.value() == 24);
}

int main(int, char*[]) {

	std::cout << "=== interop example (luwra) ===" << std::endl;
	std::cout << "code modified from luwra's documentation "
	             "examples: https://github.com/vapourismo/luwra"
	          << std::endl;

	luwra::StateWrapper state;

	state.registerUserType<ABC(int)>("ABC",
	     { LUWRA_MEMBER(ABC, value),
	          LUWRA_MEMBER(ABC, setValue) },
	     {});

	register_sol_stuff(state);

	state.runString(R"(
obj = ABC(24)
f(obj) -- call 1 argument version
f(obj, 5) -- call 2 argument version
)");

	check_with_sol(state);

	return 0;
}