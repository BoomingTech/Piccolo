#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

struct my_class {
	int a = 0;

	my_class(int x) : a(x) {
	}

	int func() {
		++a; // increment a by 1
		return a;
	}
};

int main() {

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	// Here, we are binding the member function and a class
	// instance: it will call the function on the given class
	// instance
	lua.set_function(
	     "my_class_func", &my_class::func, my_class(0));

	// We do not pass a class instance here:
	// the function will need you to pass an instance of
	// "my_class" to it in lua to work, as shown below
	lua.set_function("my_class_func_2", &my_class::func);

	// With a pre-bound instance:
	lua.script(R"(
			first_value = my_class_func()
			second_value = my_class_func()
			assert(first_value == 1)
			assert(second_value == 2)
		)");

	// With no bound instance:
	lua.set("obj", my_class(24));
	// Calls "func" on the class instance
	// referenced by "obj" in Lua
	lua.script(R"(
			third_value = my_class_func_2(obj)
			fourth_value = my_class_func_2(obj)
			assert(third_value == 25)
			assert(fourth_value == 26)
		)");

	return 0;
}
