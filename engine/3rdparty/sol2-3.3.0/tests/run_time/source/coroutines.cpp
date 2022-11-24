// sol2

// The MIT License (MIT)

// Copyright (c) 2013-2022 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "sol_test.hpp"

#include <catch2/catch_all.hpp>

#include <vector>

inline namespace sol2_test_coroutines {
	struct coroutine_thread_runner_state {
		sol::state_view* prunner_thread_state;

		coroutine_thread_runner_state(sol::state_view& ref_) : prunner_thread_state(&ref_) {
		}

		auto operator()() const {
			sol::state_view& runner_thread_state = *prunner_thread_state;
			sol::coroutine cr = runner_thread_state["loop"];
			sol::stack::push(runner_thread_state, 50);
			sol::stack::push(runner_thread_state, 25);
			int r = cr();
			return r;
		}
	};

	struct coroutine_thread_runner {
		sol::thread* prunner_thread;

		coroutine_thread_runner(sol::thread& ref_) : prunner_thread(&ref_) {
		}

		auto operator()() const {
			sol::thread& runner_thread = *prunner_thread;
			sol::state_view th_state = runner_thread.state();
			sol::coroutine cr = th_state["loop"];
			int r = cr();
			return r;
		}
	};

	struct coroutine_storage {
		coroutine_storage(sol::state& luaContext) {
			mLuaState = &luaContext;
			mThread = sol::thread::create(luaContext);
			sol::state_view luaThreadState = mThread.state();
			mThreadEnvironment = sol::environment(luaThreadState, sol::create, luaThreadState.globals());
			bool thread_environment_set_successfully = sol::set_environment(mThreadEnvironment, mThread);
			// can be either right now, since it may not have an environment depending on the given script / action
			REQUIRE((thread_environment_set_successfully || !thread_environment_set_successfully));

			sol::optional<sol::table> actionTable = luaThreadState["aTable"];
			if (actionTable) {
				mThreadTick = actionTable.value()["Tick"];
			}
		}

		coroutine_storage& operator=(coroutine_storage&& r) noexcept {
			mThread = std::move(r.mThread);
			mThreadEnvironment = std::move(r.mThreadEnvironment);
			mThreadTick = std::move(r.mThreadTick);
			return *this;
		}

		coroutine_storage(coroutine_storage&& r) noexcept
		: mLuaState(std::move(r.mLuaState))
		, mThread(std::move(r.mThread))
		, mThreadEnvironment(std::move(r.mThreadEnvironment))
		, mThreadTick(std::move(r.mThreadTick)) {
		}

		coroutine_storage& operator=(const coroutine_storage& r) = delete;

		coroutine_storage(const coroutine_storage& r) = delete;

		sol::state* mLuaState;
		sol::thread mThread;
		sol::environment mThreadEnvironment;
		sol::coroutine mThreadTick;
	};

	struct coro_h {
		int x = 500;
		int func() {
			x += 1;
			return x;
		}
	};

	struct coro_test {
		std::string identifier;
		sol::reference obj;

		coro_test(sol::this_state L, std::string id) : identifier(id), obj(L, sol::lua_nil) {
		}

		void store(sol::table ref) {
			// must be explicit
			obj = sol::reference(obj.lua_state(), ref);
		}

		void copy_store(sol::table ref) {
			// must be explicit
			obj = sol::reference(obj.lua_state(), ref);
		}

		sol::reference get() {
			return obj;
		}

		~coro_test() {
		}
	};

	struct coro_test_implicit {
		std::string identifier;
		sol::main_reference obj;

		coro_test_implicit(sol::this_state L, std::string id) : identifier(id), obj(L, sol::lua_nil) {
		}

		void store(sol::table ref) {
			// main_reference does the state shift implicitly
			obj = std::move(ref);
			lua_State* Lmain = sol::main_thread(ref.lua_state());
			REQUIRE(obj.lua_state() == Lmain);
		}

		void copy_store(sol::table ref) {
			// main_reference does the state shift implicitly
			obj = ref;
			lua_State* Lmain = sol::main_thread(ref.lua_state());
			REQUIRE(obj.lua_state() == Lmain);
		}

		sol::reference get() {
			return obj;
		}

		~coro_test_implicit() {
		}
	};
} // namespace sol2_test_coroutines

TEST_CASE("coroutines/coroutine.yield", "ensure calling a coroutine works") {
	const auto& script = R"(counter = 20
function loop()
    while counter ~= 30
    do
        coroutine.yield(counter);
        counter = counter + 1;
    end
    return counter
end
)";

	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.open_libraries(sol::lib::base, sol::lib::coroutine);
	auto result1 = lua.safe_script(script);
	REQUIRE(result1.valid());
	sol::coroutine cr = lua["loop"];

	int counter;
	for (counter = 20; counter < 31 && cr; ++counter) {
		int value = cr();
		REQUIRE(counter == value);
	}
	counter -= 1;
	REQUIRE(counter == 30);
}

TEST_CASE("coroutines/new thread coroutines", "ensure calling a coroutine works when the work is put on a different thread") {
	const auto& code = R"(counter = 20
function loop()
    while counter ~= 30
    do
        coroutine.yield(counter);
        counter = counter + 1;
    end
    return counter
end
)";

	sol::state lua;
	sol::stack_guard luasg(lua);

	lua.open_libraries(sol::lib::base, sol::lib::coroutine);
	auto result = lua.safe_script(code, sol::script_pass_on_error);
	REQUIRE(result.valid());
	sol::thread runner = sol::thread::create(lua.lua_state());
	sol::state_view runnerstate = runner.state();
	sol::coroutine cr = runnerstate["loop"];

	int counter;
	for (counter = 20; counter < 31 && cr; ++counter) {
		int value = cr();
		REQUIRE(counter == value);
	}
	counter -= 1;
	REQUIRE(counter == 30);
}

TEST_CASE("coroutines/transfer", "test that things created inside of a coroutine can have their state transferred using lua_xmove constructors") {
	for (std::size_t tries = 0; tries < 200; ++tries) {
		sol::state lua;
		sol::stack_guard luasg(lua);

		lua.open_libraries();
		{
			sol::function f2;
			lua["f"] = [&lua, &f2](sol::object t) { f2 = sol::function(lua, t); };
			{
				auto code = R"(
i = 0
function INIT()
	co = coroutine.create(
		function()
			local g = function() i = i + 1 end
			f(g)
			g = nil
			collectgarbage()
		end
	)
	coroutine.resume(co)
	co = nil
	collectgarbage()
end
)";
				auto result = lua.safe_script(code, sol::script_pass_on_error);
				REQUIRE(result.valid());
			}
			sol::function f3;
			sol::function f1;

			{
				auto code = "INIT()";
				auto result = lua.safe_script(code, sol::script_pass_on_error);
				REQUIRE(result.valid());
			}
			f2();
			auto updatecode = "return function() collectgarbage() end";
			auto pfr = lua.safe_script(updatecode);
			REQUIRE(pfr.valid());

			sol::function update = pfr.get<sol::function>();
			update();
			f3 = f2;
			f3();
			update();
			f1 = f2;
			f1();
			update();
			int i = lua["i"];
			REQUIRE(i == 3);
		}
	}
}

TEST_CASE("coroutines/explicit transfer", "check that the xmove constructors shift things around appropriately") {
	const std::string code = R"(
-- main thread - L1
-- co - L2
-- co2 - L3

x = coro_test.new("x")
local co = coroutine.wrap(
	function()
		local t = coro_test.new("t")
		local co2 = coroutine.wrap(
			function()
				local t2 = { "SOME_TABLE" }
				t:copy_store(t2) -- t2 = [L3], t.obj = [L2]
			end
		)

		co2()
		co2 = nil

		collectgarbage() -- t2 ref in t remains valid!

		x:store(t:get()) -- t.obj = [L2], x.obj = [L1]
    end
)

co()
collectgarbage()
collectgarbage()
co = nil
)";

	sol::state lua;
	lua.open_libraries(sol::lib::coroutine, sol::lib::base);

	lua.new_usertype<coro_test>("coro_test",
	     sol::constructors<coro_test(sol::this_state, std::string)>(),
	     "store",
	     &coro_test::store,
	     "copy_store",
	     &coro_test::copy_store,
	     "get",
	     &coro_test::get);

	auto r = lua.safe_script(code, sol::script_pass_on_error);
	REQUIRE(r.valid());

	coro_test& ct = lua["x"];

	lua_State* Lmain1 = lua.lua_state();
	lua_State* Lmain2 = sol::main_thread(lua);
	lua_State* Lmain3 = ct.get().lua_state();
	REQUIRE(Lmain1 == Lmain2);
	REQUIRE(Lmain1 == Lmain3);

	sol::table t = ct.get();
	REQUIRE(t.size() == 1);
	std::string s = t[1];
	REQUIRE(s == "SOME_TABLE");
}

TEST_CASE("coroutines/implicit transfer", "check that copy and move assignment constructors implicitly shift things around") {
	const std::string code = R"(
-- main thread - L1
-- co - L2
-- co2 - L3

x = coro_test.new("x")
local co = coroutine.wrap(
	function()
		local t = coro_test.new("t")
		local co2 = coroutine.wrap(
			function()
				local t2 = { "SOME_TABLE" }
				t:copy_store(t2) -- t2 = [L3], t.obj = [L2]
			end
		)

		co2()
		co2 = nil

		collectgarbage() -- t2 ref in t remains valid!

		x:store(t:get()) -- t.obj = [L2], x.obj = [L1]
    end
)

co()
collectgarbage()
collectgarbage()
co = nil
)";

	struct coro_test_implicit {
		std::string identifier;
		sol::reference obj;

		coro_test_implicit(sol::this_state L, std::string id) : identifier(id), obj(L, sol::lua_nil) {
		}

		void store(sol::table ref) {
			// must be explicit
			obj = std::move(ref);
		}

		void copy_store(sol::table ref) {
			// must be explicit
			obj = ref;
		}

		sol::reference get() {
			return obj;
		}

		~coro_test_implicit() {
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::coroutine, sol::lib::base);

	lua.new_usertype<coro_test_implicit>("coro_test",
	     sol::constructors<coro_test_implicit(sol::this_state, std::string)>(),
	     "store",
	     &coro_test_implicit::store,
	     "copy_store",
	     &coro_test_implicit::copy_store,
	     "get",
	     &coro_test_implicit::get);

	auto r = lua.safe_script(code, sol::script_pass_on_error);
	REQUIRE(r.valid());

	coro_test_implicit& ct = lua["x"];

	lua_State* Lmain1 = lua.lua_state();
	lua_State* Lmain2 = sol::main_thread(lua);
	lua_State* Lmain3 = ct.get().lua_state();
	REQUIRE(Lmain1 == Lmain2);
	REQUIRE(Lmain1 == Lmain3);

	sol::table t = ct.get();
	REQUIRE(t.size() == 1);
	std::string s = t[1];
	REQUIRE(s == "SOME_TABLE");
}

TEST_CASE("coroutines/main transfer", "check that copy and move assignment constructors using main-forced types work") {
	const std::string code = R"(
-- main thread - L1
-- co - L2
-- co2 - L3

x = coro_test.new("x")
local co = coroutine.wrap(
	function()
		local t = coro_test.new("t")
		local co2 = coroutine.wrap(
			function()
				local t2 = { "SOME_TABLE" }
				t:copy_store(t2) -- t2 = [L3], t.obj = [L2]
			end
		)

		co2()
		co2 = nil

		collectgarbage() -- t2 ref in t remains valid!

		x:store(t:get()) -- t.obj = [L2], x.obj = [L1]
    end
)

co()
co = nil
collectgarbage()
)";

	sol::state lua;
	lua.open_libraries(sol::lib::coroutine, sol::lib::base);

	lua.new_usertype<coro_test_implicit>("coro_test",
	     sol::constructors<coro_test_implicit(sol::this_state, std::string)>(),
	     "store",
	     &coro_test_implicit::store,
	     "copy_store",
	     &coro_test_implicit::copy_store,
	     "get",
	     &coro_test_implicit::get);

	auto r = lua.safe_script(code, sol::script_pass_on_error);
	REQUIRE(r.valid());

	coro_test_implicit& ct = lua["x"];

	lua_State* Lmain1 = lua.lua_state();
	lua_State* Lmain2 = sol::main_thread(lua);
	lua_State* Lmain3 = ct.get().lua_state();
	REQUIRE(Lmain1 == Lmain2);
	REQUIRE(Lmain1 == Lmain3);

	sol::table t = ct.get();
	REQUIRE(t.size() == 1);
	std::string s = t[1];
	REQUIRE(s == "SOME_TABLE");
}

TEST_CASE("coroutines/coroutine.create protection",
     "ensure that a thread picked up from coroutine.create does not throw off the lua stack entirely when called from C++") {
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::coroutine);

	auto code = R"(
function loop()
	local i = 0
	while true do
		print("pre-yield in loop")
		coroutine.yield(i)
		print("post-yield in loop")
		i = i+1
	end
end
loop_th = coroutine.create(loop)
)";

	auto r = lua.safe_script(code, sol::script_pass_on_error);
	REQUIRE(r.valid());
	sol::thread runner_thread = lua["loop_th"];

	coroutine_thread_runner test_resume(runner_thread);

	lua.set_function("test_resume", std::ref(test_resume));

	int v0 = test_resume();
	int v1 = test_resume();
	int v2, v3;
	{
		auto r2 = lua.safe_script("return test_resume()", sol::script_pass_on_error);
		REQUIRE(r2.valid());
		auto r3 = lua.safe_script("return test_resume()", sol::script_pass_on_error);
		REQUIRE(r3.valid());
		v2 = r2;
		v3 = r3;
	}
	REQUIRE(v0 == 0);
	REQUIRE(v1 == 1);
	REQUIRE(v2 == 2);
	REQUIRE(v3 == 3);
}

TEST_CASE("coroutines/stack-check", "check that resumed functions consume the entire execution stack") {
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::coroutine);
	{
		auto code = R"(
unpack = unpack or table.unpack

function loop()
    local i = 0
    while true do
        print("pre-yield in loop")
        coroutine.yield(i)
        print("post-yield in loop")
        i = i+1
    end
end
loop_th = coroutine.create(loop)
loop_res = function(...) 
	returns = { coroutine.resume(loop_th, ...) }
	return unpack(returns, 2)
end 
)";
		auto result = lua.safe_script(code, sol::script_pass_on_error);
		REQUIRE(result.valid());
	}

	// Resume from lua via thread and coroutine
	sol::thread runner_thread = lua["loop_th"];
	sol::state_view runner_thread_state = runner_thread.state();
	coroutine_thread_runner_state test_resume(runner_thread_state);
	lua.set_function("test_resume", std::ref(test_resume));

	// Resume via getting a sol::function from the state
	sol::function test_resume_lua = lua["loop_res"];

	// Resume via passing a sol::function object
	auto test_resume_func = [](sol::function f) {
		int r = f();
		return r;
	};
	lua.set_function("test_resume_func", std::ref(test_resume_func));

	int v0 = test_resume();
	int s0 = runner_thread_state.stack_top();
	int v1 = test_resume();
	int s1 = runner_thread_state.stack_top();
	int v2;
	{
		auto result = lua.safe_script("return test_resume()", sol::script_pass_on_error);
		REQUIRE(result.valid());
		v2 = result;
	}
	int s2 = runner_thread_state.stack_top();
	int v3;
	{
		auto result = lua.safe_script("return test_resume()", sol::script_pass_on_error);
		REQUIRE(result.valid());
		v3 = result;
	}
	int s3 = runner_thread_state.stack_top();
	int v4 = test_resume_lua();
	int s4 = runner_thread_state.stack_top();
	int v5;
	{
		auto result = lua.safe_script("return test_resume_func(loop_res)", sol::script_pass_on_error);
		REQUIRE(result.valid());
		v5 = result;
	}
	int s5 = runner_thread_state.stack_top();
	REQUIRE(v0 == 0);
	REQUIRE(v1 == 1);
	REQUIRE(v2 == 2);
	REQUIRE(v3 == 3);
	REQUIRE(v4 == 4);
	REQUIRE(v5 == 5);

	REQUIRE(s0 == 0);
	REQUIRE(s1 == 0);
	REQUIRE(s2 == 0);
	REQUIRE(s3 == 0);
	REQUIRE(s4 == 0);
	REQUIRE(s5 == 0);
}

TEST_CASE("coroutines/yielding", "test that a sol2 bound function can yield when marked yieldable") {
	SECTION("regular functions") {
		sol::state lua;
		lua.open_libraries(sol::lib::base, sol::lib::coroutine);

		int i = 0;
		auto func = [&i]() {
			++i;
			return i;
		};

		coro_h hobj {};

		lua["f"] = sol::yielding(func);
		lua["g"] = sol::yielding([]() { return 300; });
		lua["h"] = sol::yielding(&coro_h::func);
		lua["hobj"] = &hobj;

		sol::string_view code = R"(
		co1 = coroutine.create(function () return f() end)
		success1, value1 = coroutine.resume(co1)
		co2 = coroutine.create(function () return g() end)
		success2, value2 = coroutine.resume(co2)
		co3 = coroutine.create(function()
			h(hobj)
		end)
		success3, value3 = coroutine.resume(co3)
		)";

		auto result = lua.safe_script(code, sol::script_pass_on_error);
		REQUIRE(result.valid());

		bool success1 = lua["success1"];
		int value1 = lua["value1"];
		REQUIRE(success1);
		REQUIRE(value1 == 1);

		bool success2 = lua["success2"];
		int value2 = lua["value2"];
		REQUIRE(success2);
		REQUIRE(value2 == 300);

		bool success3 = lua["success3"];
		int value3 = lua["value3"];
		REQUIRE(success3);
		REQUIRE(value3 == 501);

		REQUIRE(hobj.x == 501);
	}
	SECTION("usertypes") {
		sol::state lua;
		lua.open_libraries(sol::lib::base, sol::lib::coroutine);

		coro_h hobj;

		lua["hobj"] = &hobj;

		lua.new_usertype<coro_h>("coro_h", "h", sol::yielding(&coro_h::func));

		sol::string_view code = R"(
		co4 = coroutine.create(function()
			hobj:h()
			hobj.h(hobj)
			coro_h.h(hobj)
		end)
		success4, value4 = coroutine.resume(co4)
		success5, value5 = coroutine.resume(co4)
		success6, value6 = coroutine.resume(co4)
		)";

		auto result = lua.safe_script(code, sol::script_pass_on_error);
		REQUIRE(result.valid());

		bool success4 = lua["success4"];
		int value4 = lua["value4"];
		REQUIRE(success4);
		REQUIRE(value4 == 501);

		bool success5 = lua["success5"];
		int value5 = lua["value5"];
		REQUIRE(success5);
		REQUIRE(value5 == 502);

		bool success6 = lua["success6"];
		int value6 = lua["value6"];
		REQUIRE(success6);
		REQUIRE(value6 == 503);

		REQUIRE(hobj.x == 503);
	}
}

TEST_CASE("coroutines/error_handler_state_transfer", "test that sol2 coroutines with their error handlers are properly sourced") {
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::coroutine);

	sol::optional<sol::error> maybe_result = lua.safe_script(R"(
		aTable = {}
		aTable["Tick"] = function()
			coroutine.yield()
		end
		)");
	REQUIRE_FALSE(maybe_result.has_value());
	int begintop = 0, endtop = 0;
	{
		test_stack_guard g(lua.lua_state(), begintop, endtop);
		std::vector<coroutine_storage> luaCoroutines;

		luaCoroutines.emplace_back(lua);
		luaCoroutines.emplace_back(lua);

		luaCoroutines[0] = std::move(luaCoroutines.back());
		luaCoroutines.pop_back();
	}
}
