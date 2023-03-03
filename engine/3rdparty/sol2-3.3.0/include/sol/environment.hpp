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

#ifndef SOL_ENVIRONMENT_HPP
#define SOL_ENVIRONMENT_HPP

#include <sol/table.hpp>

namespace sol {

	template <typename base_type>
	struct basic_environment : basic_table<base_type> {
	private:
		typedef basic_table<base_type> base_t;

	public:
		using base_t::lua_state;

		basic_environment() noexcept = default;
		basic_environment(const basic_environment&) = default;
		basic_environment(basic_environment&&) = default;
		basic_environment& operator=(const basic_environment&) = default;
		basic_environment& operator=(basic_environment&&) = default;
		basic_environment(const stack_reference& r) : basic_environment(r.lua_state(), r.stack_index()) {
		}
		basic_environment(stack_reference&& r) : basic_environment(r.lua_state(), r.stack_index()) {
		}

		basic_environment(lua_State* L, new_table nt) : base_t(L, std::move(nt)) {
		}
		template <bool b>
		basic_environment(lua_State* L, new_table t, const basic_reference<b>& fallback) : basic_environment(L, std::move(t)) {
			stack_table mt(L, new_table(0, 1));
			mt.set(meta_function::index, fallback);
			this->set(metatable_key, mt);
			mt.pop();
		}

		basic_environment(env_key_t, const stack_reference& extraction_target)
		: base_t(detail::no_safety, extraction_target.lua_state(), (stack::push_environment_of(extraction_target), -1)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			constructor_handler handler {};
			stack::check<env_key_t>(this->lua_state(), -1, handler);
#endif // Safety
			lua_pop(this->lua_state(), 1);
		}
		template <bool b>
		basic_environment(env_key_t, const basic_reference<b>& extraction_target)
		: base_t(detail::no_safety, extraction_target.lua_state(), (stack::push_environment_of(extraction_target), -1)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			constructor_handler handler {};
			stack::check<env_key_t>(this->lua_state(), -1, handler);
#endif // Safety
			lua_pop(this->lua_state(), 1);
		}
		basic_environment(lua_State* L, int index = -1) : base_t(detail::no_safety, L, index) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			constructor_handler handler {};
			stack::check<basic_environment>(L, index, handler);
#endif // Safety
		}
		basic_environment(lua_State* L, ref_index index) : base_t(detail::no_safety, L, index) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			auto pp = stack::push_pop(*this);
			constructor_handler handler {};
			stack::check<basic_environment>(L, -1, handler);
#endif // Safety
		}
		template <typename T,
		     meta::enable<meta::neg<meta::any_same<meta::unqualified_t<T>, basic_environment>>, meta::neg<std::is_same<base_type, stack_reference>>,
		          meta::neg<std::is_same<lua_nil_t, meta::unqualified_t<T>>>, is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_environment(T&& r) noexcept : base_t(detail::no_safety, std::forward<T>(r)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			if (!is_environment<meta::unqualified_t<T>>::value) {
				auto pp = stack::push_pop(*this);
				constructor_handler handler {};
				stack::check<basic_environment>(lua_state(), -1, handler);
			}
#endif // Safety
		}
		basic_environment(lua_nil_t r) noexcept : base_t(detail::no_safety, r) {
		}

		template <typename T, meta::enable<is_lua_reference<meta::unqualified_t<T>>> = meta::enabler>
		basic_environment(lua_State* L, T&& r) noexcept : base_t(detail::no_safety, L, std::forward<T>(r)) {
#if SOL_IS_ON(SOL_SAFE_REFERENCES)
			if (!is_environment<meta::unqualified_t<T>>::value) {
				auto pp = stack::push_pop(*this);
				constructor_handler handler {};
				stack::check<basic_environment>(lua_state(), -1, handler);
			}
#endif // Safety
		}

		template <typename T>
		bool set_on(const T& target) const {
			lua_State* L = target.lua_state();
			auto pp = stack::push_pop(target);
			int target_index = pp.index_of(target);
#if SOL_LUA_VERSION_I_ < 502
			// Use lua_setfenv
			this->push();
			int success_result = lua_setfenv(L, target_index);
			return success_result != 0;
#else
			// If this is a C function, the environment is always placed in
			// the first value, as is expected of sol2 (all upvalues have an empty name, "")
			if (lua_iscfunction(L, target_index) != 0) {
				const char* maybe_upvalue_name = lua_getupvalue(L, target_index, 1);
				if (maybe_upvalue_name == nullptr) {
					return false;
				}
				string_view upvalue_name(maybe_upvalue_name);
				if (upvalue_name == "") {
					this->push();
					const char* success = lua_setupvalue(L, target_index, 1);
					if (success == nullptr) {
						// left things alone on the stack, pop them off
						lua_pop(L, 2);
						return false;
					}
					lua_pop(L, 1);
					return true;
				}
				lua_pop(L, 1);
				return false;
			}
			else {
				// Must search for the right upvalue target on index
				for (int upvalue_index = 1;; ++upvalue_index) {
					const char* maybe_upvalue_name = lua_getupvalue(L, target_index, upvalue_index);
					if (maybe_upvalue_name == nullptr) {
						break;
					}
					string_view upvalue_name(maybe_upvalue_name);
					if (upvalue_name == "_ENV") {
						lua_pop(L, 1);
						this->push();
						const char* success = lua_setupvalue(L, target_index, upvalue_index);
						if (success == nullptr) {
							// left things alone on the stack, pop them off
							lua_pop(L, 1);
							break;
						}
						// whether or not we succeeded, we found _ENV
						// so we need to break
						return true;
					}
					lua_pop(L, 1);
				}
				// if we get here,
				// we did not find an _ENV here...
				return false;
			}
#endif
		}
	};

	template <typename T, typename E>
	bool set_environment(const basic_environment<E>& env, const T& target) {
		return env.set_on(target);
	}

	template <typename E = reference, typename T>
	basic_environment<E> get_environment(const T& target) {
		lua_State* L = target.lua_state();
		auto pp = stack::pop_n(L, stack::push_environment_of(target));
		return basic_environment<E>(L, -1);
	}

	struct this_environment {
		optional<environment> env;

		this_environment() : env(nullopt) {
		}
		this_environment(environment e) : env(std::move(e)) {
		}
		this_environment(const this_environment&) = default;
		this_environment(this_environment&&) = default;
		this_environment& operator=(const this_environment&) = default;
		this_environment& operator=(this_environment&&) = default;

		explicit operator bool() const {
			return static_cast<bool>(env);
		}

		operator optional<environment>&() {
			return env;
		}

		operator const optional<environment>&() const {
			return env;
		}

		operator environment&() {
			return env.value();
		}

		operator const environment&() const {
			return env.value();
		}
	};

	namespace stack {
		template <>
		struct unqualified_getter<env_key_t> {
			static environment get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return get_environment(stack_reference(L, raw_index(index)));
			}
		};

		template <>
		struct unqualified_getter<this_environment> {
			static this_environment get(lua_State* L, int, record& tracking) {
				tracking.use(0);
				lua_Debug info;
				// Level 0 means current function (this C function, which may or may not be useful for us?)
				// Level 1 means next call frame up the stack. (Can be nothing if function called directly from C++ with lua_p/call)
				int pre_stack_size = lua_gettop(L);
				if (lua_getstack(L, 1, &info) != 1) {
					if (lua_getstack(L, 0, &info) != 1) {
						lua_settop(L, pre_stack_size);
						return this_environment();
					}
				}
				if (lua_getinfo(L, "f", &info) == 0) {
					lua_settop(L, pre_stack_size);
					return this_environment();
				}

				stack_reference f(L, -1);
				environment env(env_key, f);
				if (!env.valid()) {
					lua_settop(L, pre_stack_size);
					return this_environment();
				}
				return this_environment(std::move(env));
			}
		};
	} // namespace stack
} // namespace sol

#endif // SOL_ENVIRONMENT_HPP
