table traversal keys
====================
*the definitive way to get and set things easily*


Objects ``sol::update_if_empty``, ``sol::create_if_nil``, and ``sol::override_value`` are special keys one can pass into a table traversal to enable creating tables as they go in ``nil``/empty spaces, optionally updating a value at the end of a chain of lookups if it is empty, or overriding the values and tables along a chain as they go. Each special key can be used in lookup and setting functionality on tables. It is primarily to enable easy use and creation of functionality like so:

.. literalinclude:: ../../../examples/source/table_create_if_nil.cpp
	:linenos:
