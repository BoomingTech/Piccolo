#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

// Something that can't be collided with
static const auto& script_key
     = "GlobalResource.MySpecialIdentifier123";

struct GlobalResource {
	int value = 2;
};

template <typename Handler>
bool sol_lua_check(sol::types<GlobalResource*>, lua_State* L,
     int /*index*/, Handler&& handler,
     sol::stack::record& tracking) {
	// not actually taking anything off the stack
	tracking.use(0);
	// get the field from global storage
	sol::stack::get_field<true>(L, script_key);
	// verify type
	sol::type t = static_cast<sol::type>(lua_type(L, -1));
	lua_pop(L, 1);
	if (t != sol::type::lightuserdata) {
		handler(L,
		     0,
		     sol::type::lightuserdata,
		     t,
		     "global resource is not present as a light "
		     "userdata");
		return false;
	}
	return true;
}

GlobalResource* sol_lua_get(sol::types<GlobalResource*>,
     lua_State* L, int /*index*/, sol::stack::record& tracking) {
	// retrieve the (light) userdata for this type

	// not actually pulling anything off the stack
	tracking.use(0);
	sol::stack::get_field<true>(L, script_key);
	GlobalResource* ls
	     = static_cast<GlobalResource*>(lua_touserdata(L, -1));

	// clean up stack value returned by `get_field`
	lua_pop(L, 1);
	return ls;
}

int sol_lua_push(sol::types<GlobalResource*>, lua_State* L,
     GlobalResource* ls) {
	// push light userdata
	return sol::stack::push(L, static_cast<void*>(ls));
}

int main() {

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	GlobalResource instance;

	// get GlobalResource
	lua.set_function("f", [](GlobalResource* l, int value) {
		return l->value + value;
	});
	lua.set(script_key, &instance);

	// note only 1 argument,
	// despite f having 2 arguments to recieve
	lua.script("assert(f(1) == 3)");

	return 0;
}
