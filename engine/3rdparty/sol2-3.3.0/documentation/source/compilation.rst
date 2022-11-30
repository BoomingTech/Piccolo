supported compilers, binary size, compile time
==============================================
getting good final product out of sol
-------------------------------------



supported compilers
-------------------

Newer features will be targeted at the following compilers:

* VC++
	- Visual Studio 2019, latest shipped compilers
	- Visual Studio 2018, latest shipped compilers
* GCC (includes MinGW)
	- v11.x
	- v10.x
	- v9.x
	- v8.x
	- v7.x
* Clang
	- v11.x
	- v10.x
	- v9.x
	- v8.x
	- v7.x
	- v6.x
	- v5.x
	- v4.x
	- v3.9.x

Note that Visual Studio's 2018 Community Edition is absolutely free now, and installs faster and easier than ever before. It also removes a lot of hacky work arounds and formally supports decltype SFINAE.

MinGW's GCC version 7.x of the compiler fixes a long-standing derp in the ``<codecvt>`` header that swapped the endianness of utf16 and utf32 strings.

Clang 3.4, 3.5 and 3.6 have many bugs we have run into when developing sol3 and that have negatively impacted users for a long time now.

We encourage all users to upgrade immediately. If you need old code for some reason, use `sol2 release v2.20.1`_: otherwise, always grab sol2's latest.


"I need older support"
++++++++++++++++++++++

GCC 7.x is now out alongside Visual Studio 2018. This means that `sol2 release v2.20.1`_ is the current version of the code targeted at the older compilers not listed above. Newer code will be targeted at C++17 and above.

``v2.20.1`` supports:

* VC++
	- Visual Studio 2018
	- Visual Studio 2015 (Latest updates)
* GCC (includes MinGW)
	- v7.x
	- v6.x
	- v5.x
	- v4.8+
* Clang
	- v4.x
	- v3.9.x
	- v3.8.x
	- v3.7.x
	- v3.6.x
	- Note: this applies to XCode's Apple Clang as well, but that compiler packs its own deficiencies and problems as well

If you need someone to, explicitly, backport some functionality or feature from a newer version that you like, then you can reach out to `these folks here <https://soasis.org/contact/opensource>`_.



feature support
---------------

We support C++17. We have optional support for ``<variant>`` that is turned off by default for bad compilers like AppleClang with its missing implementation.



supported Lua version
---------------------

We support:

* Lua 5.3+
* Lua 5.2
* Lua 5.1
* LuaJIT 2.0.x+
* LuaJIT 2.1.x-beta3+



binary sizes
------------

For individiauls who use :doc:`usertypes<api/usertype>` a lot, they can find their compilation times increase. This is due to C++11 and C++14 not having very good facilities for handling template parameters and variadic template parameters. There are a few things in cutting-edge C++17 and C++Next that sol can use, but the problem is many people cannot work with the latest and greatest: therefore, we have to use older techniques that result in a fair amount of redundant function specializations that can be subject to the pickiness of the compiler's inlining and other such techniques.



compile speed improvemements
----------------------------

Here are some notes on achieving better compile times without sacrificing too much performance:

* When you bind lots of usertypes, put them all in a *single* translation unit (one C++ file) so that it is not recompiled multiple times over, only to be discarded later by the linker.
	- Remember that the usertype binding ends up being serialized into the Lua state, so you never need them to appear in a header and cause that same compilation overhead for every compiled unit in your project.
* Consider placing groups of bindings in multiple different translation units (multiple C++ source files) so that only part of the bindings are recompiled when you have to change the bindings.
	- Avoid putting your bindings into headers: it *will* slow down your compilation
* If you are developing a shared library, restrict your overall surface area by specifically and explicitly marking functions as visible and exported and leaving everything else as hidden or invisible by default
* For people who already have a tool that retrieves function signatures and arguments, it might be in your best interest to hook into that tool or generator and dump out the information once using sol3's lower-level abstractions. An `issue describing preliminary steps can be found here`_.



next steps
----------

The next step for sol from a developer standpoint is to formally make the library a C++17 one. This would mean using Fold Expressions and several other things which will reduce compilation time drastically. Unfortunately, that means also boosting compiler requirements. While most wouldn't care, others are very slow to upgrade: finding the balance is difficult, and often we have to opt for backwards compatibility and fixes for bad / older compilers (of which there are many in the codebase already).

Hopefully, as things progress, we move things forward.


.. _sol2 release v2.20.1: https://github.com/ThePhD/sol2/releases/tag/v2.20.1
.. _issue describing preliminary steps can be found here: https://github.com/ThePhD/sol2/issues/436#issuecomment-312021508
