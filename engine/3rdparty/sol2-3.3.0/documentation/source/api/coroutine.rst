coroutine
=========
*resumable/yielding functions from Lua*


A ``coroutine`` is a :doc:`reference<reference>` to a function in Lua that can be called multiple times to yield a specific result. It is a cooperative function. It is run on the :doc:`lua_State<state>` that was used to create it (see :doc:`thread<thread>` for an example on how to get a coroutine that runs on a stack space separate from your usual "main" stack space :doc:`lua_State<state>`).

The ``coroutine`` object is entirely similar to the :doc:`protected_function<protected_function>` object, with additional member functions to check if a coroutine has yielded (:doc:`call_status::yielded<types>`) and is thus runnable again, whether it has completed (:ref:`call_status::ok<call-status>`) and thus cannot yield anymore values, or whether it has suffered an error (see :ref:`status()<thread-status>`'s and :ref:`call_status<call-status>`'s error codes).

For example, you can work with a coroutine like this:

.. literalinclude:: ../../../examples/source/docs/coroutine_main.cpp
    :caption: co.lua
    :name: co-lua
    :lines: 8-15
    :linenos:

This is a function that yields. We set the ``counter`` value in C++, and then use the coroutine to get a few values:

.. literalinclude:: ../../../examples/source/docs/coroutine_main.cpp
    :caption: coroutine_main.cpp
    :name: coroutine_main
    :lines: 1-6,18-19,21,25-
    :linenos:

Note that this code doesn't check for errors: to do so, you can call the function and assign it as ``auto result = loop_coroutine();``, then check ``result.valid()`` as is the case with :doc:`protected_function<protected_function>`.

Finally, you can  run this coroutine on another stack space (NOT a different computer thread: Lua uses the term 'thread' a bit strangely, as we follow its usage of the term, but it is NOT a separate thread) by doing the following:

.. literalinclude:: ../../../examples/source/docs/coroutine_thread.cpp
    :caption: coroutine_thread.cpp
    :name: yield-main-thread
    :lines: 1-6,18-19,21,25-
    :linenos:



The following are the members of ``sol::coroutine``:

members
-------

.. code-block:: cpp
    :caption: function: constructor
    :name: sol-coroutine-constructor

    coroutine(lua_State* L, int index = -1);

Grabs the coroutine at the specified index given a ``lua_State*``. 

.. code-block:: cpp
    :caption: returning the coroutine's status
    :name: sol-coroutine-status

    call_status status() const noexcept;

Returns the status of a coroutine.


.. code-block:: cpp
    :caption: checks for an error
    :name: sol-coroutine-error

    bool error() const noexcept;

Checks if an error occured when the coroutine was run.

.. _runnable:

.. code-block:: cpp
    :caption: runnable and explicit operator bool
    :name: sol-coroutine-runnable

    bool runnable () const noexcept;
    explicit operator bool() const noexcept;

These functions allow you to check if a coroutine can still be called (has more values to yield and has not errored). If you have a coroutine object ``coroutine my_co = /*...*/``, you can either check ``runnable()`` or do ``if ( my_co ) { /* use coroutine */ }``.

.. code-block:: cpp
    :caption: calling a coroutine
    :name: sol-coroutine-operator-call

    template<typename... Args>
    protected_function_result operator()( Args&&... args );

    template<typename... Ret, typename... Args>
    decltype(auto) call( Args&&... args );

    template<typename... Ret, typename... Args>
    decltype(auto) operator()( types<Ret...>, Args&&... args );

Calls the coroutine. The second ``operator()`` lets you specify the templated return types using the ``my_co(sol::types<int, std::string>, ...)`` syntax. Check ``status()`` afterwards for more information about the success of the run or just check the coroutine object in an ifs tatement, as shown :ref:`above<runnable>`.
