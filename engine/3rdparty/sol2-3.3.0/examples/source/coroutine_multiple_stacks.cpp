#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <string>
#include <iostream>

int main() {
	std::cout << "=== coroutine - multple threads ==="
	          << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::base,
	     sol::lib::package,
	     sol::lib::coroutine);

	lua["print"] = [](sol::object v) {
		std::cout << v.as<std::string>() << std::endl;
	};
	lua["cyield"] = sol::yielding(
	     []() { std::cout << "YIELDING" << std::endl; });

	// notice the new threads!
	sol::thread thread1 = sol::thread::create(lua);
	sol::thread thread2 = sol::thread::create(lua);

	// notice we load it FROM the new "execution stack"
	// we need it to have thread1's stack perspective
	sol::coroutine co1 = thread1.state().load(R"(
		print("AA : Step 1")
		cyield()
		print("AA : Step 2")
	)");
	// call first coroutine here
	co1();

	// notice we load it FROM the new "execution stack"
	// we need it to have thread2's stack and perspective
	sol::coroutine co2 = thread2.state().load(R"(
		print("BB : Step 1")
		cyield()
		print("BB : Step 2")
	)");

	// run the other coroutine
	co2();
	co1();
	// tada! they run on
	// independent stacks

	return 0;
}
