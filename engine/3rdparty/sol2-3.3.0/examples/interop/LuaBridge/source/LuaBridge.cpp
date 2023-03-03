#define SOL_ALL_SAFETIES_ON 1
#define SOL_ENABLE_INTEROP \
	1 // MUST be defined to use interop features
#include <sol/sol.hpp>

#include <LuaBridge/LuaBridge.h>

#include <iostream>

// LuaBridge,
// no longer maintained, by VinnieFalco:
// https://github.com/vinniefalco/LuaBridge

struct A {

	A(int v) : v_(v) {
	}

	void print() {
		std::cout << "called A::print" << std::endl;
	}

	int value() const {
		return v_;
	}

private:
	int v_ = 50;
};

template <typename T, typename Handler>
inline bool sol_lua_interop_check(sol::types<T>, lua_State* L,
     int relindex, sol::type index_type, Handler&& handler,
     sol::stack::record& tracking) {
	// just marking unused parameters for no compiler warnings
	(void)index_type;
	(void)handler;
	tracking.use(1);
	int index = lua_absindex(L, relindex);
	T* corrected = luabridge::Userdata::get<T>(L, index, true);
	return corrected != nullptr;
}

template <typename T>
inline std::pair<bool, T*> sol_lua_interop_get(sol::types<T> t,
     lua_State* L, int relindex, void* unadjusted_pointer,
     sol::stack::record& tracking) {
	(void)unadjusted_pointer;
	int index = lua_absindex(L, relindex);
	if (!sol_lua_interop_check(t,
	         L,
	         index,
	         sol::type::userdata,
	         sol::no_panic,
	         tracking)) {
		return { false, nullptr };
	}
	T* corrected = luabridge::Userdata::get<T>(L, index, true);
	return { true, corrected };
}

void register_sol_stuff(lua_State* L) {
	// grab raw state and put into state_view
	// state_view is cheap to construct
	sol::state_view lua(L);
	// bind and set up your things: everything is entirely
	// self-contained
	lua["f"] = sol::overload(
	     [](A& from_luabridge) {
		     std::cout << "calling 1-argument version with "
		                  "luabridge-created A { "
		               << from_luabridge.value() << " }"
		               << std::endl;
		     sol_c_assert(from_luabridge.value() == 24);
	     },
	     [](A& from_luabridge, int second_arg) {
		     std::cout << "calling 2-argument version with "
		                  "luabridge-created A { "
		               << from_luabridge.value()
		               << " } and integer argument of "
		               << second_arg << std::endl;
		     sol_c_assert(from_luabridge.value() == 24);
		     sol_c_assert(second_arg == 5);
	     });
}

void check_with_sol(lua_State* L) {
	sol::state_view lua(L);
	A& obj = lua["obj"];
	(void)obj;
	sol_c_assert(obj.value() == 24);
}

int main(int, char*[]) {

	std::cout << "=== interop example (LuaBridge) ==="
	          << std::endl;
	std::cout << "code modified from LuaBridge's examples: "
	             "https://github.com/vinniefalco/LuaBridge"
	          << std::endl;

	struct closer {
		void operator()(lua_State* L) {
			lua_close(L);
		}
	};

	std::unique_ptr<lua_State, closer> state(luaL_newstate());
	lua_State* L = state.get();
	luaL_openlibs(L);

	luabridge::getGlobalNamespace(L)
	     .beginNamespace("test")
	     .beginClass<A>("A")
	     .addConstructor<void (*)(int)>()
	     .addFunction("print", &A::print)
	     .addFunction("value", &A::value)
	     .endClass()
	     .endNamespace();

	register_sol_stuff(L);


	if (luaL_dostring(L, R"(
obj = test.A(24)
f(obj) -- call 1 argument version
f(obj, 5) -- call 2 argument version
)")) {
		lua_error(L);
	}

	check_with_sol(L);

	return 0;
}