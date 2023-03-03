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

#include <iterator>
#include <vector>
#include <list>
#include <forward_list>
#include <map>
#include <deque>
#include <array>
#include <unordered_map>
#include <set>
#include <unordered_set>


inline namespace sol2_test_container_shims {

	class int_shim {
	public:
		int_shim() = default;

		int_shim(int x) : x_(x) {
		}

		int val() const {
			return x_;
		}

	private:
		int x_ = -1;
	};

	class input_it {
	public:
		typedef std::input_iterator_tag iterator_category;
		typedef int_shim value_type;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef value_type* pointer;
		typedef std::ptrdiff_t difference_type;

		input_it() = default;

		input_it(int n, int m) : n_(n), m_(m), value_(n_) {
			assert(n_ >= 0);
			assert(m_ >= 0);
			assert(n_ <= m_);

			if (!n_ && !m_) {
				n_ = -1;
				m_ = -1;
				value_ = -1;
			}
		}

		const int_shim& operator*() const {
			return value_;
		}

		const int_shim* operator->() const {
			return &value_;
		}

		input_it& operator++() {
			assert(n_ >= 0);
			assert(m_ >= 0);
			if (n_ == m_ - 1) {
				n_ = m_ = -1;
			}
			else {
				++n_;
			}
			value_ = n_;
			return *this;
		}

		bool operator==(const input_it& i) const {
			return n_ == i.n_ && m_ == i.m_;
		}

		bool operator!=(const input_it& i) const {
			return !(*this == i);
		}

	private:
		int n_ = -1;
		int m_ = -1;
		int_shim value_;
	};

	class not_really_a_container {
	public:
		using value_type = int_shim;
		using iterator = input_it;
		using const_iterator = input_it;

		const_iterator begin() const {
			return iterator(0, 100);
		}

		const_iterator end() const {
			return iterator();
		}

		value_type gcc_warning_block() {
			return int_shim();
		}

		std::size_t size() const {
			return 100;
		}
	};

	struct my_vec : public std::vector<int> {
		typedef std::vector<int> base_t;
		using base_t::base_t;
	};

	struct order_suit {
		std::vector<std::pair<int, int64_t>> objs;
		std::vector<std::pair<int64_t, int>> objs2;

		order_suit(int pairs) {
			objs.reserve(static_cast<std::size_t>(pairs));
			objs2.reserve(static_cast<std::size_t>(pairs * 2));
			for (int i = 0; i < pairs; ++i) {
				objs.push_back({ i, i * 10 });
				objs2.push_back({ (i + pairs) * 2, (i * 2) * 50 });
				objs2.push_back({ ((i + pairs) * 2) + 1, (i * 2 + 1) * 50 });
			}
		}
	};

	class map_number_storage {

	private:
		std::unordered_map<std::string, int> data; // changed to map

	public:
		map_number_storage(int i) {
			data[std::to_string(i)] = i;
		}

		int accumulate() const // changed for map
		{
			std::size_t sum = 0;
			for (const auto& [k, v] : data) {
				(void)k;
				sum += v;
			}
			return static_cast<int>(sum);
		}

	public:
		typedef std::string key_type;
		typedef int mapped_type;
		using value_type = decltype(data)::value_type;
		using iterator = decltype(data)::iterator;
		using const_iterator = decltype(data)::const_iterator;
		using size_type = decltype(data)::size_type;

		// ADDED
		iterator find(const key_type& key) {
			return data.find(key);
		}
		auto insert(value_type kv) {
			return data.insert(kv);
		}
		auto insert(const key_type k, mapped_type v) {
			return data.insert({ k, v });
		}
		mapped_type& set(key_type k, mapped_type v) {
			return data[k] = v;
		}

		iterator begin() {
			return iterator(data.begin());
		}
		iterator end() {
			return iterator(data.end());
		}
		size_type size() const noexcept {
			return data.size();
		}
		size_type max_size() const noexcept {
			return data.max_size();
		}
		//   void push_back(int value) { data.push_back(value); }  NOT APPLICABLE TO MAP
		bool empty() const noexcept {
			return data.empty();
		}
	};

} // namespace sol2_test_container_shims

namespace sol {
	template <>
	struct is_container<my_vec> : std::true_type { };

	template <>
	struct is_container<map_number_storage> : std::false_type { };

	template <>
	struct usertype_container<my_vec> {
		// Hooks Lua's syntax for #c
		static int size(lua_State* L) {
			my_vec& v = sol::stack::get<my_vec&>(L, 1);
			return stack::push(L, v.size());
		}

		// Used by default implementation
		static auto begin(lua_State*, my_vec& self) {
			return self.begin();
		}
		static auto end(lua_State*, my_vec& self) {
			return self.end();
		}

		static std::ptrdiff_t index_adjustment(lua_State*, my_vec&) {
			return 0;
		}
	};

} // namespace sol


TEST_CASE("containers/input iterators", "test shitty input iterators that are all kinds of B L E H") {
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::package);
	lua.new_usertype<int_shim>("int_shim", "new", sol::no_constructor, "val", &int_shim::val);

	not_really_a_container c;
	lua["c"] = &c;
#if SOL_LUA_VERSION_I_ > 502
	auto result0 = lua.safe_script(R"lua(
for k, v in pairs(c) do
  assert((k - 1) == v:val())
end
)lua",
	     sol::script_pass_on_error);
	REQUIRE(result0.valid());

#endif
	auto result1 = lua.safe_script(R"lua(
for k=1,#c do
  v = c[k]
  assert((k - 1) == v:val())
end
)lua",
	     sol::script_pass_on_error);
	REQUIRE(result1.valid());
}

TEST_CASE("containers/custom indexing", "allow containers to set a custom indexing offset") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua["c"] = my_vec { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	auto result1 = lua.safe_script("for i=0,9 do assert(i == c[i]) end", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	auto result2 = lua.safe_script("assert(c[10] == nil)", sol::script_pass_on_error);
	REQUIRE(result2.valid());
	auto result3 = lua.safe_script("assert(c[-1] == nil)", sol::script_pass_on_error);
	REQUIRE(result3.valid());
	auto result4 = lua.safe_script("assert(#c == 10)", sol::script_pass_on_error);
	REQUIRE(result4.valid());
}

TEST_CASE("containers/containers of pointers", "containers of pointers shouldn't have their value_type's overly stripped") {
	sol::state lua;

	class MyContainer {
	public:
		typedef int** iterator;
		typedef int* value_type;

		std::vector<value_type> m_vec;

		inline iterator begin() {
			return m_vec.data();
		}
		inline iterator end() {
			return m_vec.data() + m_vec.size();
		}
		inline void push_back(value_type v) {
			m_vec.push_back(v);
		}
	};
	int a = 500;
	int b = 600;

	MyContainer ctr;
	ctr.push_back(&a);
	ctr.push_back(&b);
	lua["c"] = ctr;
	{
		auto result1 = lua.safe_script("ap = c[1]", sol::script_pass_on_error);
		REQUIRE(result1.valid());
		auto result2 = lua.safe_script("bp = c[2]", sol::script_pass_on_error);
		REQUIRE(result2.valid());
		int* ap = lua["ap"];
		int* bp = lua["bp"];
		REQUIRE(ap == &a);
		REQUIRE(bp == &b);
		REQUIRE(*ap == 500);
		REQUIRE(*bp == 600);
	}

	std::unordered_map<int, int*> ptrs;
	ptrs[5] = &a;
	ptrs[6] = &b;
	lua["c2"] = ptrs;
	{
		auto result1 = lua.safe_script("ap = c2[5]", sol::script_pass_on_error);
		REQUIRE(result1.valid());
		auto result2 = lua.safe_script("bp = c2[6]", sol::script_pass_on_error);
		REQUIRE(result2.valid());
		int* ap = lua["ap"];
		int* bp = lua["bp"];
		REQUIRE(ap == &a);
		REQUIRE(bp == &b);
		REQUIRE(*ap == 500);
		REQUIRE(*bp == 600);
	}
}

TEST_CASE("containers/pair container in usertypes", "make sure containers that use pairs in usertypes do not trigger compiler errors") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	auto orderSuit = lua.new_usertype<order_suit>("order_suit", sol::constructors<order_suit(int)>());
#define SET_PROP(__PROP__) orderSuit.set(#__PROP__, &order_suit::__PROP__)
	SET_PROP(objs);
	SET_PROP(objs2);
#undef SET_PROP

	auto result1 = lua.safe_script("osobj = order_suit.new(5)", sol::script_pass_on_error);
	REQUIRE(result1.valid());
	auto result2 = lua.safe_script("pvec = osobj.objs", sol::script_pass_on_error);
	REQUIRE(result2.valid());
	auto result3 = lua.safe_script("pvec2 = osobj.objs2", sol::script_pass_on_error);
	REQUIRE(result3.valid());
	using vec_t = std::remove_reference_t<decltype(std::declval<order_suit>().objs)>;
	using vec2_t = std::remove_reference_t<decltype(std::declval<order_suit>().objs2)>;
	vec_t& pvec = lua["pvec"];
	vec2_t& pvec2 = lua["pvec2"];
	REQUIRE(pvec.size() == 5);
	REQUIRE(pvec2.size() == 10);

	REQUIRE(pvec[0].first == 0);
	REQUIRE(pvec[0].second == 0);
	REQUIRE(pvec[1].first == 1);
	REQUIRE(pvec[1].second == 10);
	REQUIRE(pvec[2].first == 2);
	REQUIRE(pvec[2].second == 20);

	REQUIRE(pvec2[0].first == 10);
	REQUIRE(pvec2[0].second == 0);
	REQUIRE(pvec2[1].first == 11);
	REQUIRE(pvec2[1].second == 50);
	REQUIRE(pvec2[2].first == 12);
	REQUIRE(pvec2[2].second == 100);
	REQUIRE(pvec2[3].first == 13);
	REQUIRE(pvec2[3].second == 150);
}

TEST_CASE("containers/as_container usertype", "A usertype should be able to mark itself as a container explicitly and work with BOTH kinds of insert types") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<map_number_storage>("map_number_storage",
	     sol::constructors<map_number_storage(int)>(),
	     "accumulate",
	     &map_number_storage::accumulate,
	     "iterable",
	     [](map_number_storage& ns) {
		     return sol::as_container(ns); // treat like a container, despite is_container specialization
	     });

	sol::optional<sol::error> maybe_error0 = lua.safe_script(R"(
ns = map_number_storage.new(23)
assert(ns:accumulate() == 23)

-- reference original usertype like a container
ns_container = ns:iterable()
ns_container["24"]=24

-- now print to show effect
assert(ns:accumulate() == 47)
assert(#ns == 2)
    )",
	     &sol::script_pass_on_error);
	REQUIRE_FALSE(maybe_error0.has_value());

	map_number_storage& ns = lua["ns"];
	map_number_storage& ns_container = lua["ns_container"];

	ns.insert({ "33", 33 });

	sol::optional<sol::error> maybe_error1 = lua.safe_script(R"(
assert(ns:accumulate() == 80)
assert(#ns == 3)
assert(ns_container['33'] == 33)
    )",
	     &sol::script_pass_on_error);
	REQUIRE_FALSE(maybe_error1.has_value());

	REQUIRE(&ns == &ns_container);
	REQUIRE(ns.size() == 3);
}
