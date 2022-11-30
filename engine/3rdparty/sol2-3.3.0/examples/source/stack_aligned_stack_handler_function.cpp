#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


int main(int, char*[]) {
	sol::state lua;
	lua.script("function func (a, b) return (a + b) * 2 end");

	sol::reference func_ref = lua["func"];

	// maybe this is in a lua_CFunction you bind,
	// or maybe you're trying to work with a pre-existing system
	// maybe you've used a custom lua_load call, or you're
	// working with state_view's load(lua_Reader, ...) call...
	// here's a little bit of how you can work with the stack
	lua_State* L = lua.lua_state();

	// this is a handler:
	// stack_aligned_stack_handler,
	// as its type name explains so verbosely,
	// expects the handler on the stack
	sol::stack_reference traceback_handler(L,
	     -sol::stack::push(
	          L, sol::default_traceback_error_handler));
	// then, you need the function
	// to be on the stack
	func_ref.push();
	sol::stack_aligned_stack_handler_function func(
	     L, -1, traceback_handler);
	lua_pushinteger(L, 5); // argument 1, using plain API
	lua_pushinteger(L, 6); // argument 2

	// take 2 arguments from the top,
	// and use "stack_aligned_function" to call
	int result = func(sol::stack_count(2));
	// function call pops function and arguments,
	// leaves result on the stack for us
	// but we must manually clean the traceback handler
	// manually pop traceback handler
	traceback_handler.pop();

	// make sure everything is clean
	sol_c_assert(result == 22);
	sol_c_assert(
	     lua.stack_top() == 0); // stack is empty/balanced

	return 0;
}
