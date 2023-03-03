#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


#include <iostream>
#include <exception>
#include <fstream>

struct custom_reader {
	FILE* f;
	// We use 2 here to demonstrate
	// multiple calls to the function to
	// parse whole file.
	// Please don't use 2.
	// PLEASE DO NOT USE A BUFFER
	// OF SIZE 2!
	char buffer[2];
	std::size_t current_size;
	std::size_t read_count;

	custom_reader(FILE* f_)
	: f(f_), buffer(), current_size(0), read_count(0) {
	}

	bool read() {
		std::cout << "custom read: read #" << ++read_count
		          << std::endl;
		current_size = fread(buffer, 1, 2, f);
		return current_size > 0 && ferror(f) == 0;
	}

	~custom_reader() {
		std::fclose(f);
	}
};

// function must match signature found in type lua_Reader:
// const char* ( lua_State*, void*, size_t* )
const char* custom_reader_function(
     lua_State*, void* pointer_to_my_object, size_t* data_size) {
	custom_reader& cr
	     = *(static_cast<custom_reader*>(pointer_to_my_object));
	if (cr.read()) {
		*data_size = cr.current_size;
		return cr.buffer;
	}
	else {
		*data_size = 0;
		return nullptr;
	}
}

int main() {
	std::cout << "=== custom reader ===" << std::endl;

	// make a file to use for the custom reader
	{
		std::ofstream bjork("bjork.lua", std::ios::binary);
		bjork << "print('hello!')\n";
	}
	struct on_scope_exit {
		~on_scope_exit() {
			// remove file when done
			std::remove("bjork.lua");
		}
	} remove_on_exit;


	sol::state lua;
	lua.open_libraries(sol::lib::base);

	FILE* bjork_fp;
#ifdef _MSC_VER
	if (fopen_s(&bjork_fp, "bjork.lua", "r") != 0) {
		std::cerr << "failed to open bjork.lua -- exiting"
		          << std::endl;
		return -1;
	}
#else
	bjork_fp = fopen("bjork.lua", "r");
#endif
	if (bjork_fp == nullptr) {
		std::cerr << "failed to open bjork.lua -- exiting"
		          << std::endl;
		return -1;
	}
	custom_reader reader(bjork_fp);

	// load the code using our custom reader, then run it
	auto result = lua.safe_script(custom_reader_function,
	     &reader,
	     sol::script_pass_on_error);
	// make sure we ran loaded and ran the code successfully
	sol_c_assert(result.valid());

	// note there are lua.load( ... ) variants that take a
	// custom reader than JUST run the code, too!

	std::cout << std::endl;
	return 0;
}
