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

#ifndef SOL_TESTS_COMMON_CLASSES_HPP
#define SOL_TESTS_COMMON_CLASSES_HPP

#include <iostream>

struct woof {
	int var;

	int func(int x) {
		return var + x;
	}

	double func2(int x, int y) {
		return var + x + y + 0.5;
	}

	std::string func2s(int x, std::string y) {
		return y + " " + std::to_string(x);
	}
};

struct thing {
	int v = 100;

	thing() {
	}
	thing(int x) : v(x) {
	}
};

struct non_copyable {
	non_copyable() = default;
	non_copyable(non_copyable&& other) noexcept = default;
	non_copyable& operator=(non_copyable&& other) noexcept = default;
	non_copyable(const non_copyable& other) noexcept = delete;
	non_copyable& operator=(const non_copyable& other) noexcept = delete;
};

struct vars {
	vars() {
	}

	int boop = 0;

	~vars() {
	}
};

struct fuser {
	int x;
	fuser() : x(0) {
	}

	fuser(int x) : x(x) {
	}

	int add(int y) {
		return x + y;
	}

	int add2(int y) {
		return x + y + 2;
	}
};

namespace crapola {
	struct fuser {
		int x;
		fuser() : x(0) {
		}
		fuser(int x) : x(x) {
		}
		fuser(int x, int x2) : x(x * x2) {
		}

		int add(int y) {
			return x + y;
		}
		int add2(int y) {
			return x + y + 2;
		}
	};
} // namespace crapola

class Base {
public:
	Base(int a_num) : m_num(a_num) {
	}

	int get_num() {
		return m_num;
	}

protected:
	int m_num;
};

class Derived : public Base {
public:
	Derived(int a_num) : Base(a_num) {
	}

	int get_num_10() {
		return 10 * m_num;
	}
};

class abstract_A {
public:
	virtual void a() = 0;
	virtual ~abstract_A() {
	}
};

class abstract_B : public abstract_A {
public:
	virtual void a() override {
		std::cout << "overridden a() in B : public A - BARK" << std::endl;
	}
};

struct Vec {
	float x, y, z;
	Vec(float x, float y, float z) : x { x }, y { y }, z { z } {
	}
	float length() {
		return sqrtf(x * x + y * y + z * z);
	}
	Vec normalized() {
		float invS = 1 / length();
		return { x * invS, y * invS, z * invS };
	}
};

struct giver {
	int a = 0;

	giver() {
	}

	void gief() {
		a = 1;
	}

	static int stuff() {
		std::cout << "stuff" << std::endl;
		return 97;
	}

	static void gief_stuff(giver& t, int a) {
		t.a = a;
	}

	~giver() {
	}
};

struct lua_object {

#define MAX_INFO_STRING 64

	char info[MAX_INFO_STRING];
	const char stuck_info[MAX_INFO_STRING];

	lua_object() : info("blah"), stuck_info("solid") {
	}
};

#endif // SOL_TESTS_COMMON_CLASSES_HPP
