#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <cstddef>
#include <cstdlib>

class memory_tracker {
public:
	// 10 MB or something?
	// idk whatever
	inline static constexpr std::size_t arbitrary_default_limit
	     = 1024 * 1024 * 10;

	memory_tracker() : memory_tracker(arbitrary_default_limit) {
	}

	memory_tracker(std::size_t maximum_memory)
	: used(0)
	, limit(maximum_memory)
	, n_threads(0)
	, n_tables(0)
	, n_functions(0)
	, n_userdata(0)
	, n_strings(0) {
	}

	std::size_t currently_used() const {
		return used;
	}

	std::size_t memory_limit() const {
		return limit;
	}

	static void* allocate(void* memory_tracker_ud, void* ptr,
	     size_t object_code, size_t nsize) {
		memory_tracker& self = (*static_cast<memory_tracker*>(
		     memory_tracker_ud));
		return self.alloc(ptr, object_code, nsize);
	}

private:
	void* alloc(void* ptr, size_t original_block_size_or_code,
	     size_t new_block_size) {
		std::size_t original_block_size
		     = original_block_size_or_code;
		if (ptr == nullptr) {
			// object code!
			sol::type object_type = static_cast<sol::type>(
			     original_block_size_or_code);
			switch (object_type) {
			case sol::type::function:
				++n_functions;
				break;
			case sol::type::string:
				++n_strings;
				break;
			case sol::type::userdata:
				++n_userdata;
				break;
			case sol::type::table:
				++n_tables;
				break;
			case sol::type::thread:
				++n_threads;
				break;
			default:
				// not a clue, fam
				break;
			}
			// because it is an object code,
			// it tells us literally nothing about
			// the old block size,
			// so set that to 0
			original_block_size = 0;
		}
		if (new_block_size == 0) {
			// Lua expects us to act like a "free"
			// when the new block size is 0
			std::free(ptr);
			used -= original_block_size;
			return nullptr;
		}

		// did we hit the limit?
		std::size_t memory_differntial
		     = new_block_size - original_block_size;
		std::size_t desired_use = used + memory_differntial;
		if (desired_use > limit) {
			// tell the Lua Virtual Machine
			// to toss off (by returning nullptr)
			return nullptr;
		}
		// alright now we have to expand this shit
		// guess we use C's realloc
		ptr = std::realloc(ptr, new_block_size);
		if (ptr != nullptr) {
			// alright, we successfully allocated some space
			// track it
			used = desired_use;
		}
		// even if we're null, let Lua crash and burn for us
		return ptr;
	}

	std::size_t used;
	std::size_t limit;
	std::size_t n_threads;
	std::size_t n_tables;
	std::size_t n_functions;
	std::size_t n_userdata;
	std::size_t n_strings;
};

struct my_type {
	int a = 24;
	bool b = false;
	double d = 3.5;
};

#include <iostream>

int main() {

	std::cout << "=== memory tracker ===" << std::endl;

#if SOL_LUAJIT_VERSION < 20100 && (UINTPTR_MAX > 0xFFFFFFFF)
	std::cout << "LuaJIT in x64 mode on LuaJIT 2.0.X versions "
	             "does not support using a custom allocator!"
	          << std::endl;
#else
	memory_tracker box;
	std::cout << "memory at start: " << box.currently_used()
	          << " bytes / " << box.memory_limit() << " bytes"
	          << std::endl;
	sol::state lua(&sol::default_at_panic,
	     &memory_tracker::allocate,
	     &box);
	lua.open_libraries(sol::lib::base);

	std::cout << "memory after state creation: "
	          << box.currently_used() << " bytes / "
	          << box.memory_limit() << " bytes" << std::endl;

	// trigger some allocations
	lua.new_usertype<my_type>("my_type",
	     "a",
	     &my_type::a,
	     "b",
	     &my_type::b,
	     "d",
	     &my_type::d);
	lua["f"] = [](std::string s) { return s + " woof!"; };

	std::cout << "memory after function and usertype setup: "
	          << box.currently_used() << " bytes / "
	          << box.memory_limit() << " bytes" << std::endl;

	lua.safe_script("print(f('bark'))");
	lua.safe_script(
	     "local obj = my_type.new() print(obj.a, obj.b, obj.c) "
	     "obj.b = true print(obj.a, obj.b, obj.c)");

	std::cout << "memory at end: " << box.currently_used()
	          << " bytes / " << box.memory_limit() << " bytes"
	          << std::endl;
#endif
	return 0;
}
