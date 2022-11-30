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

#ifndef SOL_DUMP_HANDLER_HPP
#define SOL_DUMP_HANDLER_HPP

#include <sol/compatibility.hpp>

#include <cstdint>
#include <exception>

namespace sol {

	class dump_error : public error {
	private:
		int m_ec;

	public:
		dump_error(int error_code_) : error("dump returned non-zero error of " + std::to_string(error_code_)), m_ec(error_code_) {
		}

		int error_code() const {
			return m_ec;
		}
	};

	inline int dump_pass_on_error(lua_State* L_, int result_code, lua_Writer writer_function, void* userdata_pointer_, bool strip) {
		(void)L_;
		(void)writer_function;
		(void)userdata_pointer_;
		(void)strip;
		return result_code;
	}

	inline int dump_panic_on_error(lua_State* L_, int result_code, lua_Writer writer_function, void* userdata_pointer_, bool strip) {
		(void)L_;
		(void)writer_function;
		(void)userdata_pointer_;
		(void)strip;
		return luaL_error(L_, "a non-zero error code (%d) was returned by the lua_Writer for the dump function", result_code);
	}

	inline int dump_throw_on_error(lua_State* L_, int result_code, lua_Writer writer_function, void* userdata_pointer_, bool strip) {
#if SOL_IS_OFF(SOL_EXCEPTIONS)
		return dump_panic_on_error(L_, result_code, writer_function, userdata_pointer_, strip);
#else
		(void)L_;
		(void)writer_function;
		(void)userdata_pointer_;
		(void)strip;
		throw dump_error(result_code);
#endif // no exceptions stuff
	}

} // namespace sol

#endif // SOL_DUMP_HANDLER_HPP
