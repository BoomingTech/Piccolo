#define SOL_NO_EXCEPTIONS 1
#define SOL_SAFE_REFERENCES 1
#define SOL_SAFE_USERTYPE 1
#define SOL_SAFE_FUNCTION_CALLS 1
#define SOL_SAFE_FUNCTION 1
#define SOL_SAFE_NUMERICS 1
#define SOL_SAFE_GETTER 1

#include <sol/sol.hpp>
#include <iostream>

#if THIS_IS_BUSTED

std::string testFunc(std::string str) {
	std::cout << "in testFunc" << std::endl;
	return std::string("hello ") + str;
}

int main() {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua["testFunc"] = &testFunc;

	lua.script(R"(
			function f ()
                -- call a function with wrong arguments
                -- to trigger lua_error
			 print("calling function")
                testFunc()
			end
		)");

	std::cout << "bark" << std::endl;
	sol::protected_function f(lua["f"]);

	std::cout << "baa" << std::endl;
	sol::protected_function_result result = f();
	std::cout << "meow" << std::endl;
	if (result.valid()) {
		// Call succeeded
		std::cout << "call succeeded" << std::endl;
	}
	else {
		// Call failed
		sol::error err = result;
		std::string what = err.what();
		std::cout << "call failed, sol::error::what() is " << what << std::endl;
	}
}

#else
int main(int, char*[]) {
	return 0;
}
#endif
