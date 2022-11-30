#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>

int main() {

	sol::state lua;

	auto barkkey = lua["bark"];
	if (barkkey.valid()) {
		// Branch not taken: doesn't exist yet
		std::cout << "How did you get in here, arf?!"
		          << std::endl;
	}

	barkkey = 50;
	if (barkkey.valid()) {
		// Branch taken: value exists!
		std::cout << "Bark Bjork Wan Wan Wan" << std::endl;
	}

	return 0;
}
