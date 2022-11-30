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

#ifndef SOL_USERTYPE_STORAGE_HPP
#define SOL_USERTYPE_STORAGE_HPP

#include <sol/usertype_core.hpp>
#include <sol/make_reference.hpp>

#include <bitset>
#include <unordered_map>
#include <memory>

namespace sol { namespace u_detail {

	struct usertype_storage_base;
	template <typename T>
	struct usertype_storage;

	optional<usertype_storage_base&> maybe_get_usertype_storage_base(lua_State* L_, int index);
	usertype_storage_base& get_usertype_storage_base(lua_State* L_, const char* gcmetakey);
	template <typename T>
	optional<usertype_storage<T>&> maybe_get_usertype_storage(lua_State* L_);
	template <typename T>
	usertype_storage<T>& get_usertype_storage(lua_State* L_);

	using index_call_function = int(lua_State*, void*);
	using change_indexing_mem_func = void (usertype_storage_base::*)(
		lua_State*, submetatable_type, void*, stateless_stack_reference&, lua_CFunction, lua_CFunction, lua_CFunction, lua_CFunction);

	struct index_call_storage {
		index_call_function* index;
		index_call_function* new_index;
		void* binding_data;
	};

	struct new_index_call_storage : index_call_storage {
		void* new_binding_data;
	};

	struct binding_base {
		virtual void* data() = 0;
		virtual ~binding_base() {
		}
	};

	template <typename K, typename Fq, typename T = void>
	struct binding : binding_base {
		using uF = meta::unqualified_t<Fq>;
		using F = meta::conditional_t<meta::is_c_str_of_v<uF, char>
#if SOL_IS_ON(SOL_CHAR8_T)
			     || meta::is_c_str_of_v<uF, char8_t>
#endif
			     || meta::is_c_str_of_v<uF, char16_t> || meta::is_c_str_of_v<uF, char32_t> || meta::is_c_str_of_v<uF, wchar_t>,
			std::add_pointer_t<std::add_const_t<std::remove_all_extents_t<Fq>>>, std::decay_t<Fq>>;
		F data_;

		template <typename... Args>
		binding(Args&&... args) : data_(std::forward<Args>(args)...) {
		}

		virtual void* data() override {
			return static_cast<void*>(std::addressof(data_));
		}

		template <bool is_index = true, bool is_variable = false>
		static inline int call_with_(lua_State* L_, void* target) {
			constexpr int boost = !detail::is_non_factory_constructor<F>::value && std::is_same<K, call_construction>::value ? 1 : 0;
			auto& f = *static_cast<F*>(target);
			return call_detail::call_wrapped<T, is_index, is_variable, boost>(L_, f);
		}

		template <bool is_index = true, bool is_variable = false>
		static inline int call_(lua_State* L_) {
			void* f = stack::get<void*>(L_, upvalue_index(usertype_storage_index));
			return call_with_<is_index, is_variable>(L_, f);
		}

		template <bool is_index = true, bool is_variable = false>
		static inline int call(lua_State* L_) {
			int r = detail::typed_static_trampoline<decltype(&call_<is_index, is_variable>), (&call_<is_index, is_variable>)>(L_);
			if constexpr (meta::is_specialization_of_v<uF, yielding_t>) {
				return lua_yield(L_, r);
			}
			else {
				return r;
			}
		}

		template <bool is_index = true, bool is_variable = false>
		static inline int index_call_with_(lua_State* L_, void* target) {
			if constexpr (!is_variable) {
				if constexpr (is_lua_c_function_v<std::decay_t<F>>) {
					auto& f = *static_cast<std::decay_t<F>*>(target);
					return stack::push(L_, f);
				}
				else {
					// set up upvalues
					// for a chained call
					int upvalues = 0;
					upvalues += stack::push(L_, nullptr);
					upvalues += stack::push(L_, target);
					auto cfunc = &call<is_index, is_variable>;
					return stack::push(L_, c_closure(cfunc, upvalues));
				}
			}
			else {
				constexpr int boost = !detail::is_non_factory_constructor<F>::value && std::is_same<K, call_construction>::value ? 1 : 0;
				auto& f = *static_cast<F*>(target);
				return call_detail::call_wrapped<T, is_index, is_variable, boost>(L_, f);
			}
		}

		template <bool is_index = true, bool is_variable = false>
		static inline int index_call_(lua_State* L_) {
			void* f = stack::get<void*>(L_, upvalue_index(usertype_storage_index));
			return index_call_with_<is_index, is_variable>(L_, f);
		}

		template <bool is_index = true, bool is_variable = false>
		static inline int index_call(lua_State* L_) {
			int r = detail::typed_static_trampoline<decltype(&index_call_<is_index, is_variable>), (&index_call_<is_index, is_variable>)>(L_);
			if constexpr (meta::is_specialization_of_v<uF, yielding_t>) {
				return lua_yield(L_, r);
			}
			else {
				return r;
			}
		}
	};

	inline int index_fail(lua_State* L_) {
		if (lua_getmetatable(L_, 1) == 1) {
			int metatarget = lua_gettop(L_);
			stack::get_field<false, true>(L_, stack_reference(L_, raw_index(2)), metatarget);
			return 1;
		}
		// With runtime extensibility, we can't
		// hard-error things. They have to
		// return nil, like regular table types
		return stack::push(L_, lua_nil);
	}

	inline int index_target_fail(lua_State* L_, void*) {
		return index_fail(L_);
	}

	inline int new_index_fail(lua_State* L_) {
		return luaL_error(L_, "sol: cannot set (new_index) into this object: no defined new_index operation on usertype");
	}

	inline int new_index_target_fail(lua_State* L_, void*) {
		return new_index_fail(L_);
	}

	struct string_for_each_metatable_func {
		bool is_destruction = false;
		bool is_index = false;
		bool is_new_index = false;
		bool is_static_index = false;
		bool is_static_new_index = false;
		bool poison_indexing = false;
		bool is_unqualified_lua_CFunction = false;
		bool is_unqualified_lua_reference = false;
		std::string* p_key = nullptr;
		reference* p_binding_ref = nullptr;
		lua_CFunction call_func = nullptr;
		index_call_storage* p_ics = nullptr;
		usertype_storage_base* p_usb = nullptr;
		void* p_derived_usb = nullptr;
		lua_CFunction idx_call = nullptr, new_idx_call = nullptr, meta_idx_call = nullptr, meta_new_idx_call = nullptr;
		change_indexing_mem_func change_indexing;

		void operator()(lua_State* L_, submetatable_type smt_, stateless_reference& fast_index_table_) {
			std::string& key = *p_key;
			usertype_storage_base& usb = *p_usb;
			index_call_storage& ics = *p_ics;

			if (smt_ == submetatable_type::named) {
				// do not override __call or
				// other specific meta functions on named metatable:
				// we need that for call construction
				// and other amenities
				return;
			}
			int fast_index_table_push = fast_index_table_.push(L_);
			stateless_stack_reference t(L_, -fast_index_table_push);
			if (poison_indexing) {
				(usb.*change_indexing)(L_, smt_, p_derived_usb, t, idx_call, new_idx_call, meta_idx_call, meta_new_idx_call);
			}
			if (is_destruction
				&& (smt_ == submetatable_type::reference || smt_ == submetatable_type::const_reference || smt_ == submetatable_type::named
				     || smt_ == submetatable_type::unique)) {
				// gc does not apply to us here
				// for reference types (raw T*, std::ref)
				// for the named metatable itself,
				// or for unique_usertypes, which do their own custom destroyion
				t.pop(L_);
				return;
			}
			if (is_index || is_new_index || is_static_index || is_static_new_index) {
				// do not serialize the new_index and index functions here directly
				// we control those...
				t.pop(L_);
				return;
			}
			if (is_unqualified_lua_CFunction) {
				stack::set_field<false, true>(L_, key, call_func, t.stack_index());
			}
			else if (is_unqualified_lua_reference) {
				reference& binding_ref = *p_binding_ref;
				stack::set_field<false, true>(L_, key, binding_ref, t.stack_index());
			}
			else {
				stack::set_field<false, true>(L_, key, make_closure(call_func, nullptr, ics.binding_data), t.stack_index());
			}
			t.pop(L_);
		}
	};

	struct lua_reference_func {
		reference key;
		reference value;

		void operator()(lua_State* L_, submetatable_type smt_, stateless_reference& fast_index_table_) {
			if (smt_ == submetatable_type::named) {
				return;
			}
			int fast_index_table_push = fast_index_table_.push(L_);
			stateless_stack_reference t(L_, -fast_index_table_push);
			stack::set_field<false, true>(L_, key, value, t.stack_index());
			t.pop(L_);
		}
	};

	struct update_bases_func {
		detail::inheritance_check_function base_class_check_func;
		detail::inheritance_cast_function base_class_cast_func;
		lua_CFunction idx_call, new_idx_call, meta_idx_call, meta_new_idx_call;
		usertype_storage_base* p_usb;
		void* p_derived_usb;
		change_indexing_mem_func change_indexing;

		void operator()(lua_State* L_, submetatable_type smt_, stateless_reference& fast_index_table_) {
			int fast_index_table_push = fast_index_table_.push(L_);
			stateless_stack_reference t(L_, -fast_index_table_push);
			stack::set_field(L_, detail::base_class_check_key(), reinterpret_cast<void*>(base_class_check_func), t.stack_index());
			stack::set_field(L_, detail::base_class_cast_key(), reinterpret_cast<void*>(base_class_cast_func), t.stack_index());
			// change indexing, forcefully
			(p_usb->*change_indexing)(L_, smt_, p_derived_usb, t, idx_call, new_idx_call, meta_idx_call, meta_new_idx_call);
			t.pop(L_);
		}
	};

	struct binding_data_equals {
		void* binding_data;

		binding_data_equals(void* b) : binding_data(b) {
		}

		bool operator()(const std::unique_ptr<binding_base>& ptr) const {
			return binding_data == ptr->data();
		}
	};

	struct usertype_storage_base {
	public:
		lua_State* m_L;
		std::vector<std::unique_ptr<binding_base>> storage;
		std::vector<std::unique_ptr<char[]>> string_keys_storage;
		std::unordered_map<string_view, index_call_storage> string_keys;
		std::unordered_map<stateless_reference, stateless_reference, stateless_reference_hash, stateless_reference_equals> auxiliary_keys;
		stateless_reference value_index_table;
		stateless_reference reference_index_table;
		stateless_reference unique_index_table;
		stateless_reference const_reference_index_table;
		stateless_reference const_value_index_table;
		stateless_reference named_index_table;
		stateless_reference type_table;
		stateless_reference gc_names_table;
		stateless_reference named_metatable;
		new_index_call_storage base_index;
		new_index_call_storage static_base_index;
		bool is_using_index;
		bool is_using_new_index;
		std::bitset<64> properties;

		usertype_storage_base(lua_State* L_)
		: m_L(L_)
		, storage()
		, string_keys_storage()
		, string_keys()
		, auxiliary_keys(0, stateless_reference_hash(L_), stateless_reference_equals(L_))
		, value_index_table()
		, reference_index_table()
		, unique_index_table()
		, const_reference_index_table()
		, const_value_index_table()
		, named_index_table()
		, type_table(make_reference<stateless_reference>(L_, create))
		, gc_names_table(make_reference<stateless_reference>(L_, create))
		, named_metatable(make_reference<stateless_reference>(L_, create))
		, base_index()
		, static_base_index()
		, is_using_index(false)
		, is_using_new_index(false)
		, properties() {
			base_index.binding_data = nullptr;
			base_index.index = index_target_fail;
			base_index.new_index = new_index_target_fail;
			base_index.new_binding_data = nullptr;
			static_base_index.binding_data = nullptr;
			static_base_index.index = index_target_fail;
			static_base_index.new_binding_data = this;
			static_base_index.new_index = new_index_target_set;
		}

		template <typename Fx>
		void for_each_table(lua_State* L_, Fx&& fx) {
			for (int i = 0; i < 6; ++i) {
				submetatable_type smt = static_cast<submetatable_type>(i);
				stateless_reference* p_fast_index_table = nullptr;
				switch (smt) {
				case submetatable_type::const_value:
					p_fast_index_table = &this->const_value_index_table;
					break;
				case submetatable_type::reference:
					p_fast_index_table = &this->reference_index_table;
					break;
				case submetatable_type::unique:
					p_fast_index_table = &this->unique_index_table;
					break;
				case submetatable_type::const_reference:
					p_fast_index_table = &this->const_reference_index_table;
					break;
				case submetatable_type::named:
					p_fast_index_table = &this->named_index_table;
					break;
				case submetatable_type::value:
				default:
					p_fast_index_table = &this->value_index_table;
					break;
				}
				fx(L_, smt, *p_fast_index_table);
			}
		}

		void add_entry(string_view sv, index_call_storage ics) {
			string_keys_storage.emplace_back(new char[sv.size()]);
			std::unique_ptr<char[]>& sv_storage = string_keys_storage.back();
			std::memcpy(sv_storage.get(), sv.data(), sv.size());
			string_view stored_sv(sv_storage.get(), sv.size());
			string_keys.insert_or_assign(std::move(stored_sv), std::move(ics));
		}

		template <typename T, typename... Bases>
		void update_bases(lua_State* L_, bases<Bases...>) {
			static_assert(sizeof(void*) <= sizeof(detail::inheritance_check_function),
				"The size of this data pointer is too small to fit the inheritance checking function: Please file "
				"a bug report.");
			static_assert(sizeof(void*) <= sizeof(detail::inheritance_cast_function),
				"The size of this data pointer is too small to fit the inheritance checking function: Please file "
				"a bug report.");
			static_assert(!meta::any_same<T, Bases...>::value, "base classes cannot list the original class as part of the bases");
			if constexpr (sizeof...(Bases) > 0) {
				(void)detail::swallow { 0, ((weak_derive<Bases>::value = true), 0)... };

				void* derived_this = static_cast<void*>(static_cast<usertype_storage<T>*>(this));

				update_bases_func for_each_fx;
				for_each_fx.base_class_check_func = &detail::inheritance<T>::template type_check_with<Bases...>;
				for_each_fx.base_class_cast_func = &detail::inheritance<T>::template type_cast_with<Bases...>;
				for_each_fx.idx_call = &usertype_storage<T>::template index_call_with_bases<false, Bases...>;
				for_each_fx.new_idx_call = &usertype_storage<T>::template index_call_with_bases<true, Bases...>;
				for_each_fx.meta_idx_call = &usertype_storage<T>::template meta_index_call_with_bases<false, Bases...>;
				for_each_fx.meta_new_idx_call = &usertype_storage<T>::template meta_index_call_with_bases<true, Bases...>;
				for_each_fx.p_usb = this;
				for_each_fx.p_derived_usb = derived_this;
				for_each_fx.change_indexing = &usertype_storage_base::change_indexing;
				for_each_fx.p_derived_usb = derived_this;
				this->for_each_table(L_, for_each_fx);
			}
			else {
				(void)L_;
			}
		}

		void clear() {
			if (value_index_table.valid(m_L)) {
				stack::clear(m_L, value_index_table);
			}
			if (reference_index_table.valid(m_L)) {
				stack::clear(m_L, reference_index_table);
			}
			if (unique_index_table.valid(m_L)) {
				stack::clear(m_L, unique_index_table);
			}
			if (const_reference_index_table.valid(m_L)) {
				stack::clear(m_L, const_reference_index_table);
			}
			if (const_value_index_table.valid(m_L)) {
				stack::clear(m_L, const_value_index_table);
			}
			if (named_index_table.valid(m_L)) {
				stack::clear(m_L, named_index_table);
			}
			if (type_table.valid(m_L)) {
				stack::clear(m_L, type_table);
			}
			if (gc_names_table.valid(m_L)) {
				stack::clear(m_L, gc_names_table);
			}
			if (named_metatable.valid(m_L)) {
				auto pp = stack::push_pop(m_L, named_metatable);
				int named_metatable_index = pp.index_of(named_metatable);
				if (lua_getmetatable(m_L, named_metatable_index) == 1) {
					stack::clear(m_L, absolute_index(m_L, -1));
				}
				stack::clear(m_L, named_metatable);
			}

			value_index_table.reset(m_L);
			reference_index_table.reset(m_L);
			unique_index_table.reset(m_L);
			const_reference_index_table.reset(m_L);
			const_value_index_table.reset(m_L);
			named_index_table.reset(m_L);
			type_table.reset(m_L);
			gc_names_table.reset(m_L);
			named_metatable.reset(m_L);

			storage.clear();
			string_keys.clear();
			auxiliary_keys.clear();
			string_keys_storage.clear();
		}

		template <bool is_new_index, typename Base>
		static void base_walk_index(lua_State* L_, usertype_storage_base& self, bool& keep_going, int& base_result) {
			using bases = typename base<Base>::type;
			if (!keep_going) {
				return;
			}
			(void)L_;
			(void)self;
#if SOL_IS_ON(SOL_USE_UNSAFE_BASE_LOOKUP)
			usertype_storage_base& base_storage = get_usertype_storage<Base>(L_);
			base_result = self_index_call<is_new_index, true>(bases(), L_, base_storage);
#else
			optional<usertype_storage<Base>&> maybe_base_storage = maybe_get_usertype_storage<Base>(L_);
			if (static_cast<bool>(maybe_base_storage)) {
				base_result = self_index_call<is_new_index, true>(bases(), L_, *maybe_base_storage);
				keep_going = base_result == base_walking_failed_index;
			}
#endif // Fast versus slow, safe base lookup
		}

		template <bool is_new_index = false, bool base_walking = false, bool from_named_metatable = false, typename... Bases>
		static inline int self_index_call(types<Bases...>, lua_State* L, usertype_storage_base& self) {
			if constexpr (!from_named_metatable || !is_new_index) {
				type k_type = stack::get<type>(L, 2);
				if (k_type == type::string) {
					index_call_storage* target = nullptr;
					string_view k = stack::get<string_view>(L, 2);
					{
						auto it = self.string_keys.find(k);
						if (it != self.string_keys.cend()) {
							target = &it->second;
						}
					}
					if (target != nullptr) {
						// let the target decide what to do, unless it's named...
						if constexpr (is_new_index) {
							return (target->new_index)(L, target->binding_data);
						}
						else {
							return (target->index)(L, target->binding_data);
						}
					}
				}
				else if (k_type != type::lua_nil && k_type != type::none) {
					stateless_reference* target = nullptr;
					{
						stack_reference k = stack::get<stack_reference>(L, 2);
						auto it = self.auxiliary_keys.find(k);
						if (it != self.auxiliary_keys.cend()) {
							target = &it->second;
						}
					}
					if (target != nullptr) {
						if constexpr (is_new_index) {
							// set value and return
							target->reset(L, 3);
							return 0;
						}
						else {
							// push target to return
							// what we found
							return stack::push(L, *target);
						}
					}
				}
			}

			// retrieve bases and walk through them.
			bool keep_going = true;
			int base_result;
			(void)keep_going;
			(void)base_result;
			(void)detail::swallow { 1, (base_walk_index<is_new_index, Bases>(L, self, keep_going, base_result), 1)... };
			if constexpr (sizeof...(Bases) > 0) {
				if (!keep_going) {
					return base_result;
				}
			}
			if constexpr (base_walking) {
				// if we're JUST base-walking then don't index-fail, just
				// return the false bits
				return base_walking_failed_index;
			}
			else if constexpr (from_named_metatable) {
				if constexpr (is_new_index) {
					return self.static_base_index.new_index(L, self.static_base_index.new_binding_data);
				}
				else {
					return self.static_base_index.index(L, self.static_base_index.binding_data);
				}
			}
			else {
				if constexpr (is_new_index) {
					return self.base_index.new_index(L, self.base_index.new_binding_data);
				}
				else {
					return self.base_index.index(L, self.base_index.binding_data);
				}
			}
		}

		void change_indexing(lua_State* L_, submetatable_type submetatable_, void* derived_this_, stateless_stack_reference& t_, lua_CFunction index_,
			lua_CFunction new_index_, lua_CFunction meta_index_, lua_CFunction meta_new_index_) {
			usertype_storage_base& this_base = *this;
			void* base_this = static_cast<void*>(&this_base);

			this->is_using_index |= true;
			this->is_using_new_index |= true;
			if (submetatable_ == submetatable_type::named) {
				stack::set_field(L_, metatable_key, named_index_table, t_.stack_index());
				stateless_stack_reference stack_metametatable(L_, -named_metatable.push(L_));
				stack::set_field<false, true>(L_,
					meta_function::index,
					make_closure(meta_index_, nullptr, derived_this_, base_this, nullptr, toplevel_magic),
					stack_metametatable.stack_index());
				stack::set_field<false, true>(L_,
					meta_function::new_index,
					make_closure(meta_new_index_, nullptr, derived_this_, base_this, nullptr, toplevel_magic),
					stack_metametatable.stack_index());
				stack_metametatable.pop(L_);
			}
			else {
				stack::set_field<false, true>(
					L_, meta_function::index, make_closure(index_, nullptr, derived_this_, base_this, nullptr, toplevel_magic), t_.stack_index());
				stack::set_field<false, true>(
					L_, meta_function::new_index, make_closure(new_index_, nullptr, derived_this_, base_this, nullptr, toplevel_magic), t_.stack_index());
			}
		}

		template <typename T = void, typename Key, typename Value>
		void set(lua_State* L, Key&& key, Value&& value);

		static int new_index_target_set(lua_State* L, void* target) {
			usertype_storage_base& self = *static_cast<usertype_storage_base*>(target);
			self.set(L, reference(L, raw_index(2)), reference(L, raw_index(3)));
			return 0;
		}

		~usertype_storage_base() {
			value_index_table.reset(m_L);
			reference_index_table.reset(m_L);
			unique_index_table.reset(m_L);
			const_reference_index_table.reset(m_L);
			const_value_index_table.reset(m_L);
			named_index_table.reset(m_L);
			type_table.reset(m_L);
			gc_names_table.reset(m_L);
			named_metatable.reset(m_L);

			auto auxiliary_first = auxiliary_keys.cbegin();
			auto auxiliary_last = auxiliary_keys.cend();
			while (auxiliary_first != auxiliary_last) {
				// save a copy to what we're going to destroy
				auto auxiliary_target = auxiliary_first;
				// move the iterator up by 1
				++auxiliary_first;
				// extract the node and destroy the key
				auto extracted_node = auxiliary_keys.extract(auxiliary_target);
				extracted_node.key().reset(m_L);
				extracted_node.mapped().reset(m_L);
				// continue if auxiliary_first hasn't been exhausted
			}
		}
	};

	template <typename T>
	struct usertype_storage : usertype_storage_base {

		using usertype_storage_base::usertype_storage_base;

		template <bool is_new_index, bool from_named_metatable>
		static inline int index_call_(lua_State* L) {
			using bases = typename base<T>::type;
			usertype_storage_base& self = stack::get<light<usertype_storage_base>>(L, upvalue_index(usertype_storage_index));
			return self_index_call<is_new_index, false, from_named_metatable>(bases(), L, self);
		}

		template <bool is_new_index, bool from_named_metatable, typename... Bases>
		static inline int index_call_with_bases_(lua_State* L) {
			using bases = types<Bases...>;
			usertype_storage_base& self = stack::get<light<usertype_storage_base>>(L, upvalue_index(usertype_storage_index));
			return self_index_call<is_new_index, false, from_named_metatable>(bases(), L, self);
		}

		template <bool is_new_index>
		static inline int index_call(lua_State* L) {
			return detail::static_trampoline<&index_call_<is_new_index, false>>(L);
		}

		template <bool is_new_index, typename... Bases>
		static inline int index_call_with_bases(lua_State* L) {
			return detail::static_trampoline<&index_call_with_bases_<is_new_index, false, Bases...>>(L);
		}

		template <bool is_new_index>
		static inline int meta_index_call(lua_State* L) {
			return detail::static_trampoline<&index_call_<is_new_index, true>>(L);
		}

		template <bool is_new_index, typename... Bases>
		static inline int meta_index_call_with_bases(lua_State* L) {
			return detail::static_trampoline<&index_call_with_bases_<is_new_index, true, Bases...>>(L);
		}

		template <typename Key, typename Value>
		inline void set(lua_State* L, Key&& key, Value&& value);
	};

	template <typename T, typename Key, typename Value>
	void usertype_storage_base::set(lua_State* L, Key&& key, Value&& value) {
		using ValueU = meta::unwrap_unqualified_t<Value>;
		using KeyU = meta::unwrap_unqualified_t<Key>;
		using Binding = binding<KeyU, ValueU, T>;
		using is_var_bind = is_variable_binding<ValueU>;
		if constexpr (std::is_same_v<KeyU, call_construction>) {
			(void)key;
			std::unique_ptr<Binding> p_binding = std::make_unique<Binding>(std::forward<Value>(value));
			Binding& b = *p_binding;
			this->storage.push_back(std::move(p_binding));

			this->named_index_table.push(L);
			absolute_index metametatable_index(L, -1);
			std::string_view call_metamethod_name = to_string(meta_function::call);
			lua_pushlstring(L, call_metamethod_name.data(), call_metamethod_name.size());
			stack::push(L, nullptr);
			stack::push(L, b.data());
			lua_CFunction target_func = &b.template call<false, false>;
			lua_pushcclosure(L, target_func, 2);
			lua_rawset(L, metametatable_index);
			this->named_index_table.pop(L);
		}
		else if constexpr (std::is_same_v<KeyU, base_classes_tag>) {
			(void)key;
			this->update_bases<T>(L, std::forward<Value>(value));
		}
		else if constexpr ((meta::is_string_like_or_constructible<KeyU>::value || std::is_same_v<KeyU, meta_function>)) {
			std::string s = u_detail::make_string(std::forward<Key>(key));
			auto storage_it = this->storage.end();
			auto string_it = this->string_keys.find(s);
			if (string_it != this->string_keys.cend()) {
				const auto& binding_data = string_it->second.binding_data;
				storage_it = std::find_if(this->storage.begin(), this->storage.end(), binding_data_equals(binding_data));
				this->string_keys.erase(string_it);
			}

			std::unique_ptr<Binding> p_binding = std::make_unique<Binding>(std::forward<Value>(value));
			Binding& b = *p_binding;
			if (storage_it != this->storage.cend()) {
				*storage_it = std::move(p_binding);
			}
			else {
				this->storage.push_back(std::move(p_binding));
			}

			bool is_index = (s == to_string(meta_function::index));
			bool is_new_index = (s == to_string(meta_function::new_index));
			bool is_static_index = (s == to_string(meta_function::static_index));
			bool is_static_new_index = (s == to_string(meta_function::static_new_index));
			bool is_destruction = s == to_string(meta_function::garbage_collect);
			bool poison_indexing = (!is_using_index || !is_using_new_index) && (is_var_bind::value || is_index || is_new_index);
			void* derived_this = static_cast<void*>(static_cast<usertype_storage<T>*>(this));
			index_call_storage ics;
			ics.binding_data = b.data();
			ics.index = is_index || is_static_index ? &Binding::template call_with_<true, is_var_bind::value>
				                                   : &Binding::template index_call_with_<true, is_var_bind::value>;
			ics.new_index = is_new_index || is_static_new_index ? &Binding::template call_with_<false, is_var_bind::value>
				                                               : &Binding::template index_call_with_<false, is_var_bind::value>;

			string_for_each_metatable_func for_each_fx;
			for_each_fx.is_destruction = is_destruction;
			for_each_fx.is_index = is_index;
			for_each_fx.is_new_index = is_new_index;
			for_each_fx.is_static_index = is_static_index;
			for_each_fx.is_static_new_index = is_static_new_index;
			for_each_fx.poison_indexing = poison_indexing;
			for_each_fx.p_key = &s;
			for_each_fx.p_ics = &ics;
			if constexpr (is_lua_c_function_v<ValueU>) {
				for_each_fx.is_unqualified_lua_CFunction = true;
				for_each_fx.call_func = *static_cast<lua_CFunction*>(ics.binding_data);
			}
			else if constexpr (is_lua_reference_or_proxy_v<ValueU>) {
				for_each_fx.is_unqualified_lua_reference = true;
				for_each_fx.p_binding_ref = static_cast<reference*>(ics.binding_data);
			}
			else {
				for_each_fx.call_func = &b.template call<false, is_var_bind::value>;
			}
			for_each_fx.p_usb = this;
			for_each_fx.p_derived_usb = derived_this;
			for_each_fx.idx_call = &usertype_storage<T>::template index_call<false>;
			for_each_fx.new_idx_call = &usertype_storage<T>::template index_call<true>;
			for_each_fx.meta_idx_call = &usertype_storage<T>::template meta_index_call<false>;
			for_each_fx.meta_new_idx_call = &usertype_storage<T>::template meta_index_call<true>;
			for_each_fx.change_indexing = &usertype_storage_base::change_indexing;
			// set base index and base new_index
			// functions here
			if (is_index) {
				this->base_index.index = ics.index;
				this->base_index.binding_data = ics.binding_data;
			}
			if (is_new_index) {
				this->base_index.new_index = ics.new_index;
				this->base_index.new_binding_data = ics.binding_data;
			}
			if (is_static_index) {
				this->static_base_index.index = ics.index;
				this->static_base_index.binding_data = ics.binding_data;
			}
			if (is_static_new_index) {
				this->static_base_index.new_index = ics.new_index;
				this->static_base_index.new_binding_data = ics.binding_data;
			}
			this->for_each_table(L, for_each_fx);
			this->add_entry(s, std::move(ics));
		}
		else {
			// the reference-based implementation might compare poorly and hash
			// poorly in some cases...
			if constexpr (is_lua_reference_v<KeyU> && is_lua_reference_v<ValueU>) {
				if (key.get_type() == type::string) {
					stack::push(L, key);
					std::string string_key = stack::pop<std::string>(L);
					this->set<T>(L, string_key, std::forward<Value>(value));
				}
				else {
					lua_reference_func ref_additions_fx { key, value };

					this->for_each_table(L, ref_additions_fx);
					this->auxiliary_keys.insert_or_assign(std::forward<Key>(key), std::forward<Value>(value));
				}
			}
			else {
				reference ref_key = make_reference(L, std::forward<Key>(key));
				reference ref_value = make_reference(L, std::forward<Value>(value));
				lua_reference_func ref_additions_fx { ref_key, ref_value };

				this->for_each_table(L, ref_additions_fx);
				this->auxiliary_keys.insert_or_assign(std::move(ref_key), std::move(ref_value));
			}
		}
	}

	template <typename T>
	template <typename Key, typename Value>
	void usertype_storage<T>::set(lua_State* L, Key&& key, Value&& value) {
		static_cast<usertype_storage_base&>(*this).set<T>(L, std::forward<Key>(key), std::forward<Value>(value));
	}

	template <typename T>
	inline void clear_usertype_registry_names(lua_State* L) {
		using u_traits = usertype_traits<T>;
		using u_const_traits = usertype_traits<const T>;
		using u_unique_traits = usertype_traits<d::u<T>>;
		using u_ref_traits = usertype_traits<T*>;
		using u_const_ref_traits = usertype_traits<T const*>;

		stack_reference registry(L, raw_index(LUA_REGISTRYINDEX));
		registry.push();
		// eliminate all named entries for this usertype
		// in the registry (luaL_newmetatable does
		// [name] = new table
		// in registry upon creation
		stack::set_field(L, &u_traits::metatable()[0], lua_nil, registry.stack_index());
		stack::set_field(L, &u_const_traits::metatable()[0], lua_nil, registry.stack_index());
		stack::set_field(L, &u_const_ref_traits::metatable()[0], lua_nil, registry.stack_index());
		stack::set_field(L, &u_ref_traits::metatable()[0], lua_nil, registry.stack_index());
		stack::set_field(L, &u_unique_traits::metatable()[0], lua_nil, registry.stack_index());
		registry.pop();
	}

	template <typename T>
	inline int destroy_usertype_storage(lua_State* L) noexcept {
		clear_usertype_registry_names<T>(L);
		return detail::user_alloc_destroy<usertype_storage<T>>(L);
	}

	template <typename T>
	inline usertype_storage<T>& create_usertype_storage(lua_State* L) {
		const char* gcmetakey = &usertype_traits<T>::gc_table()[0];

		// Make sure userdata's memory is properly in lua first,
		// otherwise all the light userdata we make later will become invalid
		int usertype_storage_push_count = stack::push<user<usertype_storage<T>>>(L, no_metatable, L);
		stack_reference usertype_storage_ref(L, -usertype_storage_push_count);

		// create and push onto the stack a table to use as metatable for this GC
		// we create a metatable to attach to the regular gc_table
		// so that the destructor is called for the usertype storage
		int usertype_storage_metatabe_count = stack::push(L, new_table(0, 1));
		stack_reference usertype_storage_metatable(L, -usertype_storage_metatabe_count);
		// set the destroyion routine on the metatable
		stack::set_field(L, meta_function::garbage_collect, &destroy_usertype_storage<T>, usertype_storage_metatable.stack_index());
		// set the metatable on the usertype storage userdata
		stack::set_field(L, metatable_key, usertype_storage_metatable, usertype_storage_ref.stack_index());
		usertype_storage_metatable.pop();

		// set the usertype storage and its metatable
		// into the global table...
		stack::set_field<true>(L, gcmetakey, usertype_storage_ref);
		usertype_storage_ref.pop();

		// then retrieve the lua-stored version so we have a well-pinned
		// reference that does not die
		stack::get_field<true>(L, gcmetakey);
		usertype_storage<T>& target_umt = stack::pop<user<usertype_storage<T>>>(L);
		return target_umt;
	}

	inline optional<usertype_storage_base&> maybe_as_usertype_storage_base(lua_State* L, int index) {
		if (type_of(L, index) != type::lightuserdata) {
			return nullopt;
		}
		usertype_storage_base& base_storage = *static_cast<usertype_storage_base*>(stack::get<void*>(L, index));
		return base_storage;
	}

	inline optional<usertype_storage_base&> maybe_get_usertype_storage_base_inside(lua_State* L, int index) {
		// okay, maybe we're looking at a table that is nested?
		if (type_of(L, index) != type::table) {
			return nullopt;
		}
		stack::get_field(L, meta_function::storage, index);
		auto maybe_storage_base = maybe_as_usertype_storage_base(L, -1);
		lua_pop(L, 1);
		return maybe_storage_base;
	}

	inline optional<usertype_storage_base&> maybe_get_usertype_storage_base(lua_State* L, int index) {
		// If we can get the index directly as this type, go for it
		auto maybe_already_is_usertype_storage_base = maybe_as_usertype_storage_base(L, index);
		if (maybe_already_is_usertype_storage_base) {
			return maybe_already_is_usertype_storage_base;
		}
		return maybe_get_usertype_storage_base_inside(L, index);
	}


	inline optional<usertype_storage_base&> maybe_get_usertype_storage_base(lua_State* L, const char* gcmetakey) {
		stack::get_field<true>(L, gcmetakey);
		auto maybe_storage = maybe_as_usertype_storage_base(L, lua_gettop(L));
		lua_pop(L, 1);
		return maybe_storage;
	}

	inline usertype_storage_base& get_usertype_storage_base(lua_State* L, const char* gcmetakey) {
		stack::get_field<true>(L, gcmetakey);
		stack::record tracking;
		usertype_storage_base& target_umt = stack::stack_detail::unchecked_unqualified_get<user<usertype_storage_base>>(L, -1, tracking);
		lua_pop(L, 1);
		return target_umt;
	}

	template <typename T>
	inline optional<usertype_storage<T>&> maybe_get_usertype_storage(lua_State* L) {
		const char* gcmetakey = &usertype_traits<T>::gc_table()[0];
		stack::get_field<true>(L, gcmetakey);
		int target = lua_gettop(L);
		if (!stack::check<user<usertype_storage<T>>>(L, target)) {
			return nullopt;
		}
		usertype_storage<T>& target_umt = stack::pop<user<usertype_storage<T>>>(L);
		return target_umt;
	}

	template <typename T>
	inline usertype_storage<T>& get_usertype_storage(lua_State* L) {
		const char* gcmetakey = &usertype_traits<T>::gc_table()[0];
		stack::get_field<true>(L, gcmetakey);
		usertype_storage<T>& target_umt = stack::pop<user<usertype_storage<T>>>(L);
		return target_umt;
	}

	template <typename T>
	inline void clear_usertype_storage(lua_State* L) {
		using u_traits = usertype_traits<T>;

		const char* gcmetakey = &u_traits::gc_table()[0];
		stack::get_field<true>(L, gcmetakey);
		if (!stack::check<user<usertype_storage<T>>>(L)) {
			lua_pop(L, 1);
			return;
		}
		usertype_storage<T>& target_umt = stack::pop<user<usertype_storage<T>>>(L);
		target_umt.clear();

		clear_usertype_registry_names<T>(L);

		stack::set_field<true>(L, gcmetakey, lua_nil);
	}

	template <typename T, automagic_flags enrollment_flags>
	inline int register_usertype(lua_State* L_, automagic_enrollments enrollments_ = {}) {
		using u_traits = usertype_traits<T>;
		using u_const_traits = usertype_traits<const T>;
		using u_unique_traits = usertype_traits<d::u<T>>;
		using u_ref_traits = usertype_traits<T*>;
		using u_const_ref_traits = usertype_traits<T const*>;
		using uts = usertype_storage<T>;

		// always have __new_index point to usertype_storage method
		// have __index always point to regular fast-lookup
		// meta_method table
		// if __new_index is invoked, runtime-swap
		// to slow __index if necessary
		// (no speed penalty because function calls
		// are all read-only -- only depend on __index
		// to retrieve function and then call happens VIA Lua)

		// __type entry:
		// table contains key -> value lookup,
		// where key is entry in metatable
		// and value is type information as a string as
		// best as we can give it

		// name entry:
		// string that contains raw class name,
		// as defined from C++

		// is entry:
		// checks if argument supplied is of type T

		// __storage entry:
		// a light userdata pointing to the storage
		// mostly to enable this new abstraction
		// to not require the type name `T`
		// to get at the C++ usertype storage within


		// we then let typical definitions potentially override these intrinsics
		// it's the user's fault if they override things or screw them up:
		// these names have been reserved and documented since sol2

		// STEP 0: tell the old usertype (if it exists)
		// to fuck off
		clear_usertype_storage<T>(L_);

		// STEP 1: Create backing store for usertype storage
		// Pretty much the most important step.
		// STEP 2: Create Lua tables used for fast method indexing.
		// This is done inside of the storage table's constructor
		usertype_storage<T>& storage = create_usertype_storage<T>(L_);
		usertype_storage_base& base_storage = storage;
		void* light_storage = static_cast<void*>(&storage);
		void* light_base_storage = static_cast<void*>(&base_storage);


		// STEP 3: set up GC escape hatch table entirely
		storage.gc_names_table.push(L_);
		stateless_stack_reference gnt(L_, -1);
		stack::set_field(L_, submetatable_type::named, &u_traits::gc_table()[0], gnt.stack_index());
		stack::set_field(L_, submetatable_type::const_value, &u_const_traits::metatable()[0], gnt.stack_index());
		stack::set_field(L_, submetatable_type::const_reference, &u_const_ref_traits::metatable()[0], gnt.stack_index());
		stack::set_field(L_, submetatable_type::reference, &u_ref_traits::metatable()[0], gnt.stack_index());
		stack::set_field(L_, submetatable_type::unique, &u_unique_traits::metatable()[0], gnt.stack_index());
		stack::set_field(L_, submetatable_type::value, &u_traits::metatable()[0], gnt.stack_index());
		gnt.pop(L_);

		// STEP 4: add some useful information to the type table
		stateless_stack_reference stacked_type_table(L_, -storage.type_table.push(L_));
		stack::set_field(L_, "name", detail::demangle<T>(), stacked_type_table.stack_index());
		stack::set_field(L_, "is", &detail::is_check<T>, stacked_type_table.stack_index());
		stacked_type_table.pop(L_);

		// STEP 5: create and hook up metatable,
		// add intrinsics
		// this one is the actual meta-handling table,
		// the next one will be the one for
		int for_each_backing_metatable_calls = 0;
		auto for_each_backing_metatable = [&](lua_State* L_, submetatable_type smt_, stateless_reference& fast_index_table_) {
			// Pointer types, AKA "references" from C++
			const char* metakey = nullptr;
			switch (smt_) {
			case submetatable_type::const_value:
				metakey = &u_const_traits::metatable()[0];
				break;
			case submetatable_type::reference:
				metakey = &u_ref_traits::metatable()[0];
				break;
			case submetatable_type::unique:
				metakey = &u_unique_traits::metatable()[0];
				break;
			case submetatable_type::const_reference:
				metakey = &u_const_ref_traits::metatable()[0];
				break;
			case submetatable_type::named:
				metakey = &u_traits::user_metatable()[0];
				break;
			case submetatable_type::value:
			default:
				metakey = &u_traits::metatable()[0];
				break;
			}

			luaL_newmetatable(L_, metakey);
			if (smt_ == submetatable_type::named) {
				// the named table itself
				// gets the associated name value
				storage.named_metatable.reset(L_, -1);
				lua_pop(L_, 1);
				// but the thing we perform the methods on
				// is still the metatable of the named
				// table
				lua_createtable(L_, 0, 6);
			}
			stateless_stack_reference t(L_, -1);
			fast_index_table_.reset(L_, t.stack_index());
			stack::set_field<false, true>(L_, meta_function::type, storage.type_table, t.stack_index());
			// destructible? serialize default destructor here
			// otherwise, not destructible: serialize a "hey you messed up"
			switch (smt_) {
			case submetatable_type::const_reference:
			case submetatable_type::reference:
			case submetatable_type::named:
				break;
			case submetatable_type::unique:
				if constexpr (std::is_destructible_v<T>) {
					stack::set_field<false, true>(L_, meta_function::garbage_collect, &detail::unique_destroy<T>, t.stack_index());
				}
				else {
					stack::set_field<false, true>(L_, meta_function::garbage_collect, &detail::cannot_destroy<T>, t.stack_index());
				}
				break;
			case submetatable_type::value:
			case submetatable_type::const_value:
			default:
				if constexpr (std::is_destructible_v<T>) {
					stack::set_field<false, true>(L_, meta_function::garbage_collect, detail::make_destructor<T>(), t.stack_index());
				}
				else {
					stack::set_field<false, true>(L_, meta_function::garbage_collect, &detail::cannot_destroy<T>, t.stack_index());
				}
				break;
			}

			static_assert(sizeof(void*) <= sizeof(detail::inheritance_check_function),
				"The size of this data pointer is too small to fit the inheritance checking function: file a bug "
				"report.");
			static_assert(sizeof(void*) <= sizeof(detail::inheritance_cast_function),
				"The size of this data pointer is too small to fit the inheritance checking function: file a bug "
				"report.");
			stack::set_field<false, true>(L_, detail::base_class_check_key(), reinterpret_cast<void*>(&detail::inheritance<T>::type_check), t.stack_index());
			stack::set_field<false, true>(L_, detail::base_class_cast_key(), reinterpret_cast<void*>(&detail::inheritance<T>::type_cast), t.stack_index());

			auto prop_fx = detail::properties_enrollment_allowed(for_each_backing_metatable_calls, storage.properties, enrollments_);
			auto insert_fx = [&L_, &t, &storage](meta_function mf, lua_CFunction reg) {
				stack::set_field<false, true>(L_, mf, reg, t.stack_index());
				storage.properties[static_cast<std::size_t>(mf)] = true;
			};
			detail::insert_default_registrations<T>(insert_fx, prop_fx);

			// There are no variables, so serialize the fast function stuff
			// be sure to reset the index stuff to the non-fast version
			// if the user ever adds something later!
			if (smt_ == submetatable_type::named) {
				// add escape hatch storage pointer and gc names
				stack::set_field<false, true>(L_, meta_function::storage, light_base_storage, t.stack_index());
				stack::set_field<false, true>(L_, meta_function::gc_names, storage.gc_names_table, t.stack_index());

				// fancy new_indexing when using the named table
				{
					absolute_index named_metatable_index(L_, -storage.named_metatable.push(L_));
					stack::set_field<false, true>(L_, metatable_key, t, named_metatable_index);
					storage.named_metatable.pop(L_);
				}
				stack_reference stack_metametatable(L_, -storage.named_index_table.push(L_));
				stack::set_field<false, true>(L_,
					meta_function::index,
					make_closure(uts::template meta_index_call<false>, nullptr, light_storage, light_base_storage, nullptr, toplevel_magic),
					stack_metametatable.stack_index());
				stack::set_field<false, true>(L_,
					meta_function::new_index,
					make_closure(uts::template meta_index_call<true>, nullptr, light_storage, light_base_storage, nullptr, toplevel_magic),
					stack_metametatable.stack_index());
				stack_metametatable.pop();
			}
			else {
				// otherwise just plain for index,
				// and elaborated for new_index
				stack::set_field<false, true>(L_, meta_function::index, t, t.stack_index());
				stack::set_field<false, true>(L_,
					meta_function::new_index,
					make_closure(uts::template index_call<true>, nullptr, light_storage, light_base_storage, nullptr, toplevel_magic),
					t.stack_index());
				storage.is_using_new_index = true;
			}

			++for_each_backing_metatable_calls;
			fast_index_table_.reset(L_, t.stack_index());
			t.pop(L_);
		};

		storage.for_each_table(L_, for_each_backing_metatable);

		// can only use set AFTER we initialize all the metatables
		if constexpr (std::is_default_constructible_v<T> && has_flag(enrollment_flags, automagic_flags::default_constructor)) {
			if (enrollments_.default_constructor) {
				storage.set(L_, meta_function::construct, constructors<T()>());
			}
		}

		// return the named metatable we want names linked into
		storage.named_metatable.push(L_);
		return 1;
	}
}} // namespace sol::u_detail

#endif // SOL_USERTYPE_STORAGE_HPP
