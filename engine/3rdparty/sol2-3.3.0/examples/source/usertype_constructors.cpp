#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <string>
#include <iostream>

class MyClass {
public:
	MyClass(double d) : data(d) {
	}
	double data;
};

int main() {
	std::cout << "=== usertype constructors ===" << std::endl;


	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<MyClass>("MyClass",
	     sol::meta_function::construct,
	     sol::factories(
	          // MyClass.new(...) -- dot syntax, no "self" value
	          // passed in
	          [](const double& d) {
		          return std::make_shared<MyClass>(d);
	          },
	          // MyClass:new(...) -- colon syntax, passes in the
	          // "self" value as first argument implicitly
	          [](sol::object, const double& d) {
		          return std::make_shared<MyClass>(d);
	          }),
	     // MyClass(...) syntax, only
	     sol::call_constructor,
	     sol::factories([](const double& d) {
		     return std::make_shared<MyClass>(d);
	     }),
	     "data",
	     &MyClass::data);

	sol::optional<sol::error> maybe_error = lua.safe_script(R"(
		d1 = MyClass(2.1)
		d2 = MyClass:new(3.1)
		d3 = MyClass(4.1)
		assert(d1.data == 2.1)
		assert(d2.data == 3.1)
		assert(d3.data == 4.1)
	)",
	     sol::script_pass_on_error);

	if (maybe_error) {
		// something went wrong!
		std::cerr << "Something has gone horribly unexpected "
		             "and wrong:\n"
		          << maybe_error->what() << std::endl;
		return 1;
	}

	// everything is okay!
	std::cout
	     << "Everything went okay and all the asserts passed!"
	     << std::endl;

	return 0;
}
