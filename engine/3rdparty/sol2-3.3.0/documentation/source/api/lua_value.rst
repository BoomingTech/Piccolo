lua_value
=========
*easy creation of Lua values and tables at the cost of some safety and speed*

.. code-block:: cpp

	struct lua_value;
	struct array_value;
	

The goal of these types is to make it easy to describe tables and arrays in C++ code. It works by using a thread local ``lua_State*`` variable inside the class so that one can simply pass values. The thread local variable is initialized by creation of a `sol::state`, but can also `be done manually<state-automatic-handlers>` with ``sol::set_default_state``. An example of usage is below:

.. literalinclude:: ../../../examples/source/lua_value.cpp
	:caption: lua_value.cpp
	:name: lua-value-example
	:linenos:
