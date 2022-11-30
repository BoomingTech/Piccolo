#define SOL_CHECK_ARGUMENTS 1
#include <sol/sol.hpp>

#include <cstdio>

bool is_file_std_out(sol::table data) {
	sol::object maybe_file = data["file"];
	if (maybe_file.is<luaL_Stream&>()) {
		luaL_Stream& filestream = data["file"];
		return filestream.f == stdout;
	}
	return false;
}

int main() {
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::io);

	lua["is_std_out"] = &is_file_std_out;
	lua.script("assert(is_std_out{ file = io.stdout })");

	return 0;
}