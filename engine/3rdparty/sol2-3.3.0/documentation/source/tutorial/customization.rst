adding your own types
=====================

Sometimes, overriding sol to make it handle certain ``struct``'s and ``class``'es as something other than just userdata is desirable. The way to do this is to take advantage of the 4 customization points for sol. These are ``sol_lua_check``, ``sol_lua_get``, ``sol_lua_push``, and ``sol_lua_check_get``.

These are template class/structs, so you'll override them using a technique C++ calls *class/struct specialization*. Below is an example of a struct that gets broken apart into 2 pieces when going in the C++ --> Lua direction, and then pulled back into a struct when going in the Lua --> C++:

.. literalinclude:: ../../../examples/source/customization_multiple.cpp
	:name: customization-overriding
	:caption: main.cpp
	:linenos:
	:lines: 1-52

This is the base formula that you can follow to extend to your own classes. Using it in the rest of the library should then be seamless:

.. literalinclude:: ../../../examples/source/customization_multiple.cpp
	:name: customization-overriding-use
	:caption: main.cpp
	:linenos:
	:lines: 52-

And that's it!

A few things of note about the implementation: First, there's an auxiliary parameter of type :doc:`sol::stack::record<../api/stack>` for the getters and checkers. This keeps track of what the last complete operation performed. Since we retrieved 2 members, we use ``tracking.use(2);`` to indicate that we used 2 stack positions (one for ``bool``, one for ``int``). The second thing to note here is that we made sure to use the ``index`` parameter, and then proceeded to add 1 to it for the next one.

You can make something pushable into Lua, but not get-able in the same way if you only specialize one part of the system. If you need to retrieve it (as a return using one or multiple values from Lua), you should specialize the ``sol::stack::getter`` template class and the ``sol::stack::checker`` template class. If you need to push it into Lua at some point, then you'll want to specialize the ``sol::stack::pusher`` template class. The ``sol::lua_size`` template class trait needs to be specialized for both cases, unless it only pushes 1 item, in which case the default implementation will assume 1.

.. note::

	It is important to note here that the ``gett``, ``push`` and ``check`` differentiate between a type ``T`` and a pointer to a type ``T*``. This means that if you want to work purely with, say, a ``T*`` handle that does not have the same semantics as just ``T``, you may need to specify checkers/getters/pushers for both ``T*`` and ``T``. The checkers for ``T*`` forward to the checkers for ``T``, but the getter for ``T*`` does not forward to the getter for ``T`` (e.g., because of ``int*`` not being quite the same as ``int``).

In general, this is fine since most getters/checkers only use 1 stack point. But, if you're doing more complex nested classes, it would be useful to use ``tracking.last`` to understand how many stack indices the last gett/check operation did and increment it by ``index + tracking.last`` after using a ``stack::check<..>( L, index, tracking)`` call.

You can read more about the extension points themselves :ref:`over on the API page for stack<extension_points>`, and if there's something that goes wrong or you have anymore questions, please feel free to drop a line on the Github Issues page or send an e-mail!
