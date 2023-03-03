usertype<T>
===========
*structures and classes from C++ made available to Lua code*


.. note::
	
	``T`` refers to the type being turned into a usertype.

.. code-block:: cpp

	class metatable : public table;

	template <typename T>
	class usertype : public metatable;

While other frameworks extend lua's syntax or create Data Structure Languages (DSLs) to create classes in Lua, :doc:`sol<../index>` instead offers the ability to generate easy bindings that pile on performance. You can see a `small starter example here`_. These use metatables and userdata in Lua for their implementation. Usertypes are also `runtime extensible`_.

There are more advanced use cases for how to create and use a usertype, which are all based on how to use its `.set()` function and its initial construction (see below).

enumerations
------------

.. _meta_function_enum:

.. code-block:: cpp
	:caption: meta_function enumeration for names
	:linenos:

	enum class meta_function {
		construct,
		index,
		new_index,
		mode,
		call,
		call_function = call,
		metatable,
		to_string,
		length,
		unary_minus,
		addition,
		subtraction,
		multiplication,
		division,
		modulus,
		power_of,
		involution = power_of,
		concatenation,
		equal_to,
		less_than,
		less_than_or_equal_to,
		garbage_collect,
		floor_division,
		bitwise_left_shift,
		bitwise_right_shift,
		bitwise_not,
		bitwise_and,
		bitwise_or,
		bitwise_xor,
		pairs,
		ipairs,
		next,
		type,
		type_info,
		call_construct,
		storage,
		gc_names,
		static_index,
		static_new_index,
	};

	typedef meta_function meta_method;

Use this enumeration to specify names in a manner friendlier than memorizing the special lua metamethod names for each of these. Each binds to a specific operation indicated by the descriptive name of the enum. You can read more about `the metamethods in the Lua manual`_ and learn about how they work and are supposed to be implemented there. Each of the names here (except for the ones used as shortcuts to other names like ``meta_function::call_function`` and ``meta_function::involution`` and not including ``construct``, which just maps to the name ``new``) link directly to the Lua name for the operation. ``meta_function::pairs`` is only available in Lua 5.2 and above (does not include LuaJIT or Lua 5.1) and ``meta_function::ipairs`` is only available in Lua 5.2 exactly (disregarding compatibiltiy flags).

Some are also sol2 specific, for example ``meta_function::type_info``, ``meta_function::call_construct``, ``meta_function::static_index`` and ``meta_function::static_new_index`` are sol2-specific and usable by users. The entries ``meta_function::storage`` and ``meta_function::gc_names`` are sol2-internal but still in the enumeration; **please** do not use them.

``meta_function::index`` and ``meta_function::new_index`` apply strictly to when an object in Lua is called with a key *it does not already know* (e.g., was not bound by the C++ programmer with ``.set(...)`` or ``.new_usertype<...>(...);``. ``meta_function::static_index`` and `meta_function::static_new_index`` functions get called when the the key is not found and the user is calling the new function from the named metatable itself.

structs
-------

.. _automagic_enrollments:

.. code-block:: cpp
	:caption: automagic_enrollments for special members defined on a class
	:linenos:	

	struct automagic_enrollments {
		bool default_constructor = true;
		bool destructor = true;
		bool pairs_operator = true;
		bool to_string_operator = true;
		bool call_operator = true;
		bool less_than_operator = true;
		bool less_than_or_equal_to_operator = true;
		bool length_operator = true;
		bool equal_to_operator = true;
	};

This structure is used with ``new_usertype`` to specifically ordain certain special member functions to be bound to Lua, whether it is capable of them or not.


new_usertype/set
----------------

``sol::usertype<T>`` is a specialized version of ``sol::metatable``\s, which are a specialized version of ``sol::table``. ``sol::metatable``\s attempt to treat the table like either a Lua or a sol2 metatable. ``sol::usertype<T>`` demands that a usertype is a specific metatable for a specific class. Both of them are `sol::reference derived types<reference>`, meaning they take in the ``lua_State*``. For example...


new_usertype/set options
++++++++++++++++++++++++

If  the type is `default_constructible`_, sol will generate a ``"new"`` member on the usertype for you. Otherwise, use ``my_table.new_usertype<MyType>("name", sol::no_constructor);`` to prevent the constructor or pass in a ``sol::automagic_enrollments enrollments; /* modify members here */;`` when calling ``.new_usertype<MyType>("name", enrollments);``. Otherwise, the following are special ways to handle the construction of a usertype:
 
..  _constructor:

* ``"{name}", constructors<T(), T(arg-1-0), T(arg-2-0, arg-2-1), ...>``
	- Specify the constructors to be bound under ``name``: list constructors by specifying their function signature with ``class_type(arg0, arg1, ... argN)``
	- If you pass the ``constructors<...>`` argument first when constructing the usertype, then it will automatically be given a ``"{name}"`` of ``"new"``
* ``"{name}", constructors<Type-List-0, Type-List-1, ...>``
	- This syntax is longer and provided for backwards-compatibility: the above argument syntax is shorter and cleaner
	- ``Type-List-N`` must be a ``sol::types<Args...>``, where ``Args...`` is a list of types that a constructor takes. Supports overloading by default
	- If you pass the ``constructors<...>`` argument first when constructing the usertype, then it will automatically be given a ``"{name}"`` of ``"new"``
* ``"{name}", sol::initializers( func1, func2, ... )``
	- Used to handle *initializer functions* that need to initialize the memory itself (but not actually allocate the memory, since that comes as a userdata block from Lua)
	- Given one or more functions, provides an overloaded Lua function for creating the specified type
		+ The function must have the argument signature ``func( T*, Arguments... )`` or ``func( T&, Arguments... )``, where the pointer or reference will point to a place of allocated memory that has an uninitialized ``T``. Note that Lua controls the memory, so performing a ``new`` and setting it to the ``T*`` or ``T&`` is a bad idea: instead, use ``placement new`` to invoke a constructor, or deal with the memory exactly as you see fit
* ``{anything}, sol::factories( func1, func2, ... )``
	- Used to indicate that a factory function (e.g., something that produces a ``std::unique_ptr<T, ...>``, ``std::shared_ptr<T>``, ``T``, or similar) will be creating the object type
	- Given one or more functions, provides an overloaded function for invoking
		+ The functions can take any form and return anything, since they're just considered to be some plain function and no placement new or otherwise needs to be done. Results from this function will be pushed into Lua according to the same rules as everything else.
		+ Can be used to stop the generation of a ``.new()`` default constructor since a ``sol::factories`` entry will be recognized as a constructor for the usertype
		+ If this is not sufficient, see next 2 entries on how to specifically block a constructor
* ``{anything}, {some_factory_function}``
	- Essentially binds whatever the function is to name ``{anything}``
	- When used WITH the ``sol::no_constructor`` option below (e.g. ``"new", sol::no_constructor`` and after that having ``"create", &my_creation_func``), one can remove typical constructor avenues and then only provide specific factory functions. Note that this combination is similar to using the ``sol::factories`` method mentioned earlier in this list. To control the destructor as well, see further below
* ``sol::call_constructor, {valid constructor / initializer / factory}``
	- The purpose of this is to enable the syntax ``local v = my_class( 24 )`` and have that call a constructor; it has no other purpose
	- This is compatible with luabind, kaguya and other Lua library syntaxes and looks similar to C++ syntax, but the general consensus in Programming with Lua and other places is to use a function named ``new``
	- Note that with the ``sol::call_constructor`` key, a construct type above must be specified. A free function without it will pass in the metatable describing this object as the first argument without that distinction, which can cause strange runtime errors.
* ``{anything}, sol::no_constructor``
	- Specifically tells sol not to create a ``.new()`` if one is not specified and the type is default-constructible
	- When the key ``{anything}`` is called on the table, it will result in an error. The error might be that the type is not-constructible. 
	- *Use this plus some of the above to allow a factory function for your function type but prevent other types of constructor idioms in Lua*

usertype destructor options
+++++++++++++++++++++++++++

If you don't specify anything at all and the type is `destructible`_, then a destructor will be bound to the garbage collection metamethod. Otherwise, the following are special ways to handle the destruction of a usertype:

* ``"__gc", sol::destructor( func )`` or ``sol::meta_function::garbage_collect, sol::destructor( func )``
	- Creates a custom destructor that takes an argument ``T*`` or ``T&`` and expects it to be destructed/destroyed. Note that lua controls the memory and thusly will deallocate the necessary space AFTER this function returns (e.g., do not call ``delete`` as that will attempt to deallocate memory you did not ``new``)
	- If you just want the default constructor, you can replace the second argument with ``sol::default_destructor``
	- The usertype will error / throw if you specify a destructor specifically but do not map it to ``sol::meta_function::gc`` or a string equivalent to ``"__gc"``

.. note::

	You MUST specify ``sol::destructor`` around your destruction function, otherwise it will be ignored.


.. _automagical-registration:

usertype automatic (automagic) meta functions
+++++++++++++++++++++++++++++++++++++++++++++

If you don't specify a ``sol::meta_function`` name (or equivalent string metamethod name) and the type ``T`` supports certain operations, sol2 will generate the following operations provided it can find a good default implementation:

* for ``to_string`` operations where ``std::ostream& operator<<( std::ostream&, const T& )``, ``obj.to_string()``, or ``to_string( const T& )`` (in the namespace) exists on the C++ type
	- a ``sol::meta_function::to_string`` operator will be generated
	- writing is done into either
		+ a ``std::ostringstream`` before the underlying string is serialized into Lua
		+ directly serializing the return value of ``obj.to_string()`` or ``to_string( const T& )``
	- order of preference is the ``std::ostream& operator<<``, then the member function ``obj.to_string()``, then the ADL-lookup based ``to_string( const T& )``
	- if you need to turn this behavior off for a type (for example, to avoid compiler errors for ADL conflicts), specialize ``sol::is_to_stringable<T>`` for your type to be ``std::false_type``, like so:

.. code-block:: cpp

	namespace sol {
		template <>
		struct is_to_stringable<my_type> : std::false_type {};
	}


* for call operations where ``operator()( parameters ... )`` exists on the C++ type
	- a ``sol::meta_function::call`` operator will be generated
	- the function call operator in C++ must not be overloaded, otherwise sol will be unable to bind it automagically
	- the function call operator in C++ must not be templated, otherwise sol will be unable to bind it automagically
	- if it is overloaded or templated, it is your responsibility to bind it properly
* for automatic iteration where ``begin()`` and ``end()`` functions exist on the C++ type
	- the type **must** have a ``value_type``, ``iterator``, and `similar Container typedefs`_.
	- a ``sol::meta_function::pairs`` operator is generated for you
	- Allows you to iterate using ``for k, v in pairs( obj ) do ... end`` in Lua
	- **Lua 5.2 and better only: LuaJIT does not allow this, Lua 5.1 does NOT allow this**
* for cases where ``.size()`` exists on the C++ type
	- the length operator of Lua (``#my_obj``) operator is generated for you
* for comparison operations where ``operator <`` and ``operator <=`` exist on the C++ type 
	- These two ``sol::meta_function::less_than(_or_equal_to)`` are generated for you
	- ``>`` and ``>=`` operators are generated in Lua based on ``<`` and ``<=`` operators
* for ``operator==``
	- An equality operator will always be generated, doing pointer comparison if ``operator==`` on the two value types is not supported or doing a reference comparison and a value comparison if ``operator==`` is supported
* heterogenous operators cannot be supported for equality, as Lua specifically checks if they use the same function to do the comparison: if they do not, then the equality method is not invoked; one way around this would be to write one ``int super_equality_function(lua_State* L) { ... }``, pull out arguments 1 and 2 from the stack for your type, and check all the types and then invoke ``operator==`` yourself after getting the types out of Lua (possibly using :ref:`sol::stack::get<stack-get>` and :ref:`sol::stack::check_get<stack-check-get>`)



usertype regular function options
+++++++++++++++++++++++++++++++++

Otherwise, the following is used to specify functions to bind on the specific usertype for ``T``.

* ``"{name}", &free_function``
	- Binds a free function / static class function / function object (lambda) to ``"{name}"``. If the first argument is ``T*`` or ``T&``, then it will bind it as a member function. If it is not, it will be bound as a "static" function on the lua table
* ``"{name}", &type::function_name`` or ``"{name}", &type::member_variable``
	- Binds a typical member function or variable to ``"{name}"``. In the case of a member variable or member function, ``type`` must be ``T`` or a base of ``T``
* ``"{name}", sol::readonly( &type::member_variable )``
	- Binds a typical variable to ``"{name}"``. Similar to the above, but the variable will be read-only, meaning an error will be generated if anything attemps to write to this variable
* ``"{name}", sol::as_function( &type::member_variable )``
	- Binds a typical variable to ``"{name}"`` *but forces the syntax to be callable like a function*. This produces a getter and a setter accessible by ``obj:name()`` to get and ``obj:name(value)`` to set.
* ``"{name}", sol::property( getter_func, setter_func )``
	- Binds a typical variable to ``"{name}"``, but gets and sets using the specified setter and getter functions. Not that if you do not pass a setter function, the variable will be read-only. Also not that if you do not pass a getter function, it will be write-only
* ``"{name}", sol::var( some_value )`` or ``"{name}", sol::var( std::ref( some_value ) )``
	- Binds a typical variable to ``"{name}"``, optionally by reference (e.g., refers to the same memory in C++). This is useful for global variables / static class variables and the like
* ``"{name}", sol::overload( Func1, Func2, ... )``
	- Creates an oveloaded member function that discriminates on number of arguments and types
	- Dumping multiple functions out with the same name **does not make an overload**: you must use **this syntax** in order for it to work
* ``sol::base_classes, sol::bases<Bases...>``
	- Tells a usertype what its base classes are. You need this to have derived-to-base conversions work properly. See :ref:`inheritance<usertype-inheritance>`


unregister
----------

You can unlink and kill a usertype and its associated functionality by calling ``.unregister()`` on a ``sol::usertype<T>`` or ``sol::metatable`` pointed at a proper sol2 metatable. This will entirely unlink and clean out sol2's internal lookup structures and key information.

runtime functions
-----------------

You can add functions at runtime **to the whole class** (not to individual objects). Set a name under the metatable name you bound using ``new_usertype`` to an object. For example:

.. literalinclude:: ../../../examples/source/docs/runtime_extension.cpp
	:caption: runtime_extension.cpp
	:name: runtime-extension-example
	:linenos:

.. note::

	You cannot add functions to an individual object. You can only add functions to the whole class / usertype.


overloading
-----------

Functions set on a usertype support overloading. See :doc:`here<overload>` for an example.


.. _usertype-inheritance:

inheritance
-----------

sol can adjust pointers from derived classes to base classes at runtime, but it has some caveats based on what you compile with:

If your class has no complicated™ virtual inheritance or multiple inheritance, than you can try to sneak away with a performance boost from not specifying any base classes and doing any casting checks. (What does "complicated™" mean? Ask your compiler's documentation, if you're in that deep.)

For the rest of us safe individuals out there: You must specify the ``sol::base_classes`` tag with the ``sol::bases<Types...>()`` argument, where ``Types...`` are all the base classes of the single type ``T`` that you are making a usertype out of.

Register the base classes explicitly.

.. note::

	Always specify your bases if you plan to retrieve a base class using the sol abstraction directly and not casting yourself.

.. literalinclude:: ../../../examples/source/docs/inheritance.cpp
	:caption: inheritance.cpp
	:name: inheritance-example
	:linenos:
	:emphasize-lines: 23

.. note::

	You must list ALL base classes, including (if there were any) the base classes of A, and the base classes of those base classes, etc. if you want sol/Lua to handle them automagically.

.. note::
	
	sol does not support down-casting from a base class to a derived class at runtime.

.. warning::

	Specify all base class member variables and member functions to avoid current implementation caveats regarding automatic base member lookup. sol currently attempts to link base class methods and variables with their derived classes with an undocumented, unsupported feature, provided you specify ``sol::bases<...>``. Unfortunately, this can come at the cost of performance, depending on how "far" the base is from the derived class in the bases lookup list. If you do not want to suffer the performance degradation while we iron out the kinks in the implementation (and want it to stay performant forever), please specify all the base methods on the derived class in the method listing you write. In the future, we hope that with reflection we will not have to worry about this.

.. _automagical:

automagical usertypes
---------------------

Usertypes automatically register special functions, whether or not they're bound using `new_usertype`. You can turn this off by specializing the ``sol::is_automagical<T>`` template trait:

.. code-block:: cpp

	struct my_strange_nonconfirming_type { /* ... */ };

	namespace sol {
		template <>
		struct is_automagical<my_strange_nonconforming_type> : std::false_type {};
	}

inheritance + overloading
-------------------------

While overloading is supported regardless of inheritance caveats or not, the current version of sol has a first-match, first-call style of overloading when it comes to inheritance. Put the functions with the most derived arguments first to get the kind of matching you expect or cast inside of an intermediary C++ function and call the function you desire.

compilation speed
-----------------

.. note::

	MSVC and clang/gcc may need additional compiler flags to handle compiling extensive use of usertypes. See: :ref:`the error documentation<compilation_errors_warnings>` for more details.

performance note
----------------

.. note::

	Note that performance for member function calls goes down by a fixed overhead if you also bind variables as well as member functions. This is purely a limitation of the Lua implementation and there is, unfortunately, nothing that can be done about it. If you bind only functions and no variables, however, sol will automatically optimize the Lua runtime and give you the maximum performance possible. *Please consider ease of use and maintenance of code before you make everything into functions.*

.. _destructible: http://en.cppreference.com/w/cpp/types/is_destructible
.. _default_constructible: http://en.cppreference.com/w/cpp/types/is_constructible
.. _small starter example here: https://github.com/ThePhD/sol2/blob/develop/examples/source/usertype_basics.cpp
.. _runtime extensible: https://github.com/ThePhD/sol2/blob/develop/examples/source/usertype_advanced.cpp#L81
.. _the metamethods in the Lua manual: https://www.lua.org/manual/5.3/manual.html#2.4
.. _similar Container typedefs: http://eel.is/c++draft/container.requirements.general#4
