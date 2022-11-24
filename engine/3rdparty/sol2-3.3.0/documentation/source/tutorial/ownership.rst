ownership
=========

Ownership is important when managing resources in C++. sol has many ownership semantics which are generally safe by default. Below are the rules.

object ownership
----------------

You can take a reference to something that exists in Lua by pulling out a :doc:`sol::reference<../api/reference>` or a :doc:`sol::object<../api/object>`:

.. literalinclude:: ../../../examples/source/tutorials/object_lifetime.cpp
	:linenos:
	:caption: object_lifetime.cpp

All objects must be destroyed before the `sol::state` is destroyed, otherwise you will end up with dangling references to the Lua State and things will explode in horrible, terrible fashion.

This applies to more than just `sol::object`: all types derived from `sol::reference` and `sol::object` (:doc:`sol::table<../api/table>` :doc:`sol::userdata<../api/userdata>`, etc.) must be cleaned up before the state goes out of scope.


pointer ownership
-----------------

sol will not take ownership of raw pointers: raw pointers do not own anything. sol will not delete raw pointers, because they do not (and are not supposed to) own anything:

.. literalinclude:: ../../../examples/source/tutorials/pointer_lifetime.cpp
	:linenos:
	:caption: pointer_lifetime.cpp
	:name: pointer-lifetime-raw-ptr
	:lines: 1-11,14-22, 74-

Use/return a ``unique_ptr`` or ``shared_ptr`` instead or just return a value:

.. literalinclude:: ../../../examples/source/tutorials/pointer_lifetime.cpp
	:linenos:
	:caption: (smart pointers) pointer_lifetime.cpp
	:name: pointer-lifetime-smart-ptr
	:lines: 1-11,25-44, 74-

If you have something you know is going to last and you just want to give it to Lua as a reference, then it's fine too:

.. literalinclude:: ../../../examples/source/tutorials/pointer_lifetime.cpp
	:linenos:
	:caption: (static) pointer_lifetime.cpp
	:name: pointer-lifetime-static
	:lines: 1-11,46-49,74-


sol can detect ``nullptr``, so if you happen to return it there won't be any dangling because a ``sol::lua_nil`` will be pushed. But if you know it's ``nil`` beforehand, please return ``std::nullptr_t`` or ``sol::lua_nil``:

.. literalinclude:: ../../../examples/source/tutorials/pointer_lifetime.cpp
	:linenos:
	:caption: (nil/nullptr) pointer_lifetime.cpp
	:name: pointer-lifetime-nil
	:lines: 1-11,51-


ephermeal (proxy) objects
-------------------------

:doc:`Proxy<../api/proxy>` and result types are ephermeal. They rely on the Lua stack and their constructors / destructors interact with the Lua stack. This means they are entirely unsafe to return from functions in C++, without very careful attention paid to how they are used that often requires relying on implementation-defined behaviors.

Please be careful when using `(protected_)function_result`, `load_result` (especially multiple load/function results in a single C++ function!) `stack_reference`, and similar stack-based things. If you want to return these things, consider 
