#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <string>
#include <iostream>

// shows how to load basic data to a struct

struct config {
	std::string name;
	int width;
	int height;

	void print() {
		std::cout << "Name: " << name << '\n'
		          << "Width: " << width << '\n'
		          << "Height: " << height << '\n';
	}
};

int main() {
	sol::state lua;
	config screen;
	// To use the file, uncomment here and make sure it is in
	// local dir lua.script_file("config.lua");
	lua.script(R"(
name = "Asus"
width = 1920
height = 1080
)");
	screen.name = lua.get<std::string>("name");
	screen.width = lua.get<int>("width");
	screen.height = lua.get<int>("height");
	sol_c_assert(screen.name == "Asus");
	sol_c_assert(screen.width == 1920);
	sol_c_assert(screen.height == 1080);

	std::cout << "=== config ===" << std::endl;
	screen.print();
	std::cout << std::endl;
}
