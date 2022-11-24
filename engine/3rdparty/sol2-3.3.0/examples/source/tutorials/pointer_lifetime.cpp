#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

struct my_type {
	void stuff() {
	}
};

int main() {

	sol::state lua;

	/*
	// AAAHHH BAD
	// dangling pointer!
	lua["my_func"] = []() -> my_type* { return new my_type(); };

	// AAAHHH!
	lua.set("something", new my_type());

	// AAAAAAHHH!!!
	lua["something_else"] = new my_type();
	*/

	// :ok:
	lua["my_func0"] = []() -> std::unique_ptr<my_type> {
		return std::make_unique<my_type>();
	};

	// :ok:
	lua["my_func1"] = []() -> std::shared_ptr<my_type> {
		return std::make_shared<my_type>();
	};

	// :ok:
	lua["my_func2"] = []() -> my_type { return my_type(); };

	// :ok:
	lua.set(
	     "something", std::unique_ptr<my_type>(new my_type()));

	std::shared_ptr<my_type> my_shared
	     = std::make_shared<my_type>();
	// :ok:
	lua.set("something_else", my_shared);

	// :ok:
	auto my_unique = std::make_unique<my_type>();
	lua["other_thing"] = std::move(my_unique);

	// :ok:
	lua["my_func5"] = []() -> my_type* {
		static my_type mt;
		return &mt;
	};

	// THIS IS STILL BAD DON'T DO IT AAAHHH BAD
	// return a unique_ptr that's empty instead
	// or be explicit!
	lua["my_func6"] = []() -> my_type* { return nullptr; };

	// :ok:
	lua["my_func7"]
	     = []() -> std::nullptr_t { return nullptr; };

	// :ok:
	lua["my_func8"] = []() -> std::unique_ptr<my_type> {
		// default-constructs as a nullptr,
		// gets pushed as nil to Lua
		return std::unique_ptr<my_type>();
		// same happens for std::shared_ptr
	};

	// Acceptable, it will set 'something' to nil
	// (and delete it on next GC if there's no more references)
	lua.set("something", nullptr);

	// Also fine
	lua["something_else"] = nullptr;

	return 0;
}
