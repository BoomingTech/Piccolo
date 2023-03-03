functions and You
=================

sol can register all kinds of functions. Many are shown in the :doc:`quick 'n' dirty<all-the-things>`, but here we will discuss many of the additional ways you can register functions into a sol-wrapped Lua system.

Setting a new function
----------------------

Given a C++ function, you can drop it into sol in several equivalent ways, working similar to how :ref:`setting variables<writing-variables-demo>` works:

.. literalinclude:: ../../../examples/source/tutorials/writing_functions.cpp
	:language: cpp
	:linenos:
	:name: writing-functions
	:caption: Registering C++ functions.

The same code works with all sorts of functions, from member function/variable pointers you have on a class as well as lambdas:

.. literalinclude:: ../../../examples/source/tutorials/writing_member_functions.cpp
	:language: cpp
	:linenos:
	:caption: Registering C++ member functions
	:name: writing-member-functions



Member class functions and member class variables will both be turned into functions when set in this manner. You can get intuitive variable with the ``obj.a = value`` access after this section when you learn about :doc:`usertypes to have C++ in Lua<cxx-in-lua>`, but for now we're just dealing with functions!


Another question a lot of people have is about function templates. Function templates -- member functions or free functions -- cannot be registered because they do not exist until you instantiate them in C++. Therefore, given a templated function such as:

.. literalinclude:: ../../../examples/source/tutorials/writing_template_functions.cpp
	:language: cpp
	:linenos:
	:lines: 4-7
	:caption: A C++ templated function
	:name: writing-templated-functions-my_add

You must specify all the template arguments in order to bind and use it, like so:

.. literalinclude:: ../../../examples/source/tutorials/writing_template_functions.cpp
	:language: cpp
	:linenos:
	:lines: 7-
	:caption: Registering function template instantiations
	:name: writing-templated-functions

Notice here that we bind two separate functions. What if we wanted to bind only one function, but have it behave differently based on what arguments it is called with? This is called Overloading, and it can be done with :doc:`sol::overload<../api/overload>` like so:

.. literalinclude:: ../../../examples/source/tutorials/writing_overloaded_template_functions.cpp
	:language: cpp
	:linenos:
	:caption: Registering C++ function template instantiations
	:name: writing-templated-functions-overloaded

This is useful for functions which can take multiple types and need to behave differently based on those types. You can set as many overloads as you want, and they can be of many different types.

.. note::

	Binding functions with default parameters (``void func(int a = 1);``) does not magically bind multiple versions of the function to be called with the default parameters. You must instead use :doc:`sol::overload<../api/overload>` and bind the full version of the function, with lambdas or similar for the function calls one by one.

.. note::

	please make sure to understand the :ref:`implications of binding a lambda/callable struct in the various ways <binding-callable-objects>` and what it means for your code!


Getting a function from Lua
---------------------------

There are 2 ways to get a function from Lua. One is with :doc:`sol::unsafe_function<../api/function>` and the other is a more advanced wrapper with :doc:`sol::protected_function<../api/protected_function>`. As the names indicate, one is safer and allows you to introspect on errors during the function's execution, while the other assumes that everything ran fine and simply provides the result (for an often negligible increase in speed).

You can use them to retrieve callables from Lua and call the underlying function, in two ways:

.. literalinclude:: ../../../examples/source/tutorials/reading_functions.cpp
	:language: cpp
	:linenos:
	:caption: Retrieving a sol::(unsafe\ _/protected\ _)function
	:name: reading-functions

.. note::

	``sol::function`` is an alias for either ``sol::protected_function`` or ``sol::unsafe_function``, depending on the :doc:`configuration settings </safety>`.

You can get anything that's a callable in Lua, including C++ functions bound using ``set_function`` or similar. ``sol::protected_function`` behaves similarly to ``sol::unsafe_function``, but has a :ref:`set_error_handler() <protected-function-error-handler>` variable you can set to a Lua function. This catches all errors and runs them through the error-handling function:


.. code-block:: cpp
	:linenos:
	:caption: Retrieving a sol::protected_function 
	:name: reading-protected-functions

	int main () {
		sol::state lua;

		lua.script(R"(
			function handler (message)
				return "Handled this message: " .. message
			end

			function f (a)
				if a < 0 then
					error("negative number detected")
				end
				return a + 5
			end
		)");

		sol::protected_function f = lua["f"];
		f.set_error_handler(lua["handler"]);

		sol::protected_function_result result = f(-500);
		if (result.valid()) {
			// Call succeeded
			int x = result;
		}
		else {
			// Call failed
			sol::error err = result;
			std::string what = err.what();
			// 'what' Should read 
			// "Handled this message: negative number detected"
		} 
	}


Multiple returns to and from Lua
--------------------------------

You can return multiple items to and from Lua using ``std::tuple``/``std::pair`` classes provided by C++. These enable you to also use :doc:`sol::tie<../api/tie>` to set return values into pre-declared items. To recieve multiple returns, just ask for a ``std::tuple`` type from the result of a function's computation, or ``sol::tie`` a bunch of pre-declared variables together and set the result equal to that:

.. code-block:: cpp
	:linenos:
	:caption: Multiple returns from Lua 
	:name: multi-return-lua-functions	

	int main () {
		sol::state lua;

		lua.script("function f (a, b, c) return a, b, c end");
		
		std::tuple<int, int, int> result;
		result = lua["f"](1, 2, 3); 
		// result == { 1, 2, 3 }
		int a, int b;
		std::string c;
		sol::tie( a, b, c ) = lua["f"](1, 2, "bark");
		// a == 1
		// b == 2
		// c == "bark"
	}

You can also return mutiple items yourself from a C++-bound function. Here, we're going to bind a C++ lambda into Lua, and then call it through Lua and get a ``std::tuple`` out on the other side:

.. code-block:: cpp	
	:linenos:
	:caption: Multiple returns into Lua 
	:name: multi-return-cxx-functions	

	int main () {
		sol::state lua;

		lua["f"] = [](int a, int b, sol::object c) {
			// sol::object can be anything here: just pass it through
			return std::make_tuple( a, b, c );
		};
		
		std::tuple<int, int, int> result = lua["f"](1, 2, 3); 
		// result == { 1, 2, 3 }
		
		std::tuple<int, int, std::string> result2;
		result2 = lua["f"](1, 2, "Arf?")
		// result2 == { 1, 2, "Arf?" }

		int a, int b;
		std::string c;
		sol::tie( a, b, c ) = lua["f"](1, 2, "meow");
		// a == 1
		// b == 2
		// c == "meow"
	}


Note here that we use :doc:`sol::object<../api/object>` to transport through "any value" that can come from Lua. You can also use ``sol::make_object`` to create an object from some value, so that it can be returned into Lua as well.


Any return to and from Lua
--------------------------

It was hinted at in the previous code example, but ``sol::object`` is a good way to pass "any type" back into Lua (while we all wait for ``std::variant<...>`` to get implemented and shipped by C++ compiler/library implementers).

It can be used like so, inconjunction with ``sol::this_state``:

.. code-block:: cpp	
	:linenos:
	:caption: Return anything into Lua 
	:name: object-return-cxx-functions	

	sol::object fancy_func (sol::object a, sol::object b, sol::this_state s) {
		sol::state_view lua(s);
		if (a.is<int>() && b.is<int>()) {
			return sol::make_object(lua, a.as<int>() + b.as<int>());
		}
		else if (a.is<bool>()) {
			bool do_triple = a.as<bool>();
			return sol::make_object(lua, b.as<double>() * ( do_triple ? 3 : 1 ) );
		}
		return sol::make_object(lua, sol::lua_nil);
	}

	int main () {
		sol::state lua;

		lua["f"] = fancy_func;
		
		int result = lua["f"](1, 2);
		// result == 3
		double result2 = lua["f"](false, 2.5);
		// result2 == 2.5

		// call in Lua, get result
		lua.script("result3 = f(true, 5.5)");
		double result3 = lua["result3"];
		// result3 == 16.5
	}


This covers almost everything you need to know about Functions and how they interact with sol. For some advanced tricks and neat things, check out :doc:`sol::this_state<../api/this_state>` and :doc:`sol::variadic_args<../api/variadic_args>`. The next stop in this tutorial is about :doc:`C++ types (usertypes) in Lua<cxx-in-lua>`! If you need a bit more information about functions in the C++ side and how to best utilize arguments from C++, see :ref:`this note<function-argument-handling>`.
