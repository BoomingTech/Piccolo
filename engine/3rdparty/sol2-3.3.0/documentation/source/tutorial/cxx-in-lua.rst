C++ in Lua
==========

Using user defined types ("usertype"s, or just "udt"s) is simple with sol. If you don't call any member variables or functions, then you don't even have to 'register' the usertype at all: just pass it through. But if you want variables and functions on your usertype inside of Lua, you need to register it. We're going to give a short example here that includes a bunch of information on how to work with things.

Take this ``player`` struct in C++ in a header file:

.. literalinclude:: ../../../examples/source/usertype_advanced.cpp
	:caption: player.hpp
	:name: tutorial-player-class
	:linenos:
	:lines: 8-51

It's a fairly minimal class, but we don't want to have to rewrite this with metatables in Lua. We want this to be part of Lua easily. The following is the Lua code that we'd like to have work properly:

.. literalinclude:: ../../../examples/source/usertype_advanced.cpp
	:caption: player_script.lua
	:name: tutorial-player-script
	:language: lua
	:linenos:
	:lines: 97-127

To do this, you bind things using the ``new_usertype`` and method as shown below. These methods are on both :doc:`table<../api/table>` and :doc:`state(_view)<../api/state>`, but we're going to just use it on ``state``:

.. literalinclude:: ../../../examples/source/usertype_advanced.cpp
	:caption: main.cpp
	:name: tutorial-player-usertype
	:language: cpp
	:linenos:
	:lines: 1-3,5,7-9,53,55-85,135-136,143-

There is one more method used in the script that is not in C++ or defined on the C++ code to bind a usertype, called ``brake``. Even if a method does not exist in C++, you can add methods to the *class table* in Lua:

.. literalinclude:: ../../../examples/source/usertype_advanced.cpp
	:caption: prelude_script.lua
	:name: tutorial-prelude-script
	:language: lua
	:linenos:
	:lines: 89-92

That script should run fine now, and you can observe and play around with the values. Even more stuff :doc:`you can do<../api/usertype>` is described elsewhere, like initializer functions (private constructors / destructors support), "static" functions callable with ``name.my_function( ... )``, and overloaded member functions. You can even bind global variables (even by reference with ``std::ref``) with ``sol::var``. There's a lot to try out!

This is a powerful way to allow reuse of C++ code from Lua beyond just registering functions, and should get you on your way to having more complex classes and data structures! In the case that you need more customization than just usertypes, however, you can customize sol to behave more fit to your desires by using the desired :doc:`customization and extension structures<customization>`.

You can check out this code and more complicated code at the `examples directory`_ by looking at the ``usertype_``-prefixed examples.

.. _examples directory: https://github.com/ThePhD/sol2/tree/develop/examples