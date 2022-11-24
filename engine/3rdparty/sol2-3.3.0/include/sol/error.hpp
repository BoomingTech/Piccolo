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

#ifndef SOL_ERROR_HPP
#define SOL_ERROR_HPP

#include <sol/compatibility.hpp>

#include <stdexcept>
#include <string>
#include <array>

namespace sol {
	namespace detail {
		struct direct_error_tag { };
		const auto direct_error = direct_error_tag {};

		struct error_result {
			int results;
			const char* format_string;
			std::array<const char*, 4> argument_strings;

			error_result() : results(0), format_string(nullptr) {
			}

			error_result(int results_) : results(results_), format_string(nullptr) {
			}

			error_result(const char* format_string_, const char* first_message_) : results(0), format_string(format_string_), argument_strings() {
				argument_strings[0] = first_message_;
			}
		};

		inline int handle_errors(lua_State* L, const error_result& er) {
			if (er.format_string == nullptr) {
				return er.results;
			}
			return luaL_error(L, er.format_string, er.argument_strings[0], er.argument_strings[1], er.argument_strings[2], er.argument_strings[3]);
		}
	} // namespace detail

	class error : public std::runtime_error {
	private:
		// Because VC++ is upsetting, most of the time!
		std::string what_reason;

	public:
		error(const std::string& str) : error(detail::direct_error, "lua: error: " + str) {
		}
		error(std::string&& str) : error(detail::direct_error, "lua: error: " + std::move(str)) {
		}
		error(detail::direct_error_tag, const std::string& str) : std::runtime_error(""), what_reason(str) {
		}
		error(detail::direct_error_tag, std::string&& str) : std::runtime_error(""), what_reason(std::move(str)) {
		}

		error(const error& e) = default;
		error(error&& e) = default;
		error& operator=(const error& e) = default;
		error& operator=(error&& e) = default;

		virtual const char* what() const noexcept override {
			return what_reason.c_str();
		}
	};

} // namespace sol

#endif // SOL_ERROR_HPP
