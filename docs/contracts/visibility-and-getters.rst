.. index:: ! visibility, external, public, private, internal

.. _visibility-and-getters:

**********************
Visibility and Getters
**********************

Solidity knows two kinds of function calls: internal
ones that do not create an actual EVM call (also called
a "message call") and external
ones that do. Because of that, there are four types of visibility for
functions and state variables.

Functions have to be specified as being ``external``,
``public``, ``internal`` or ``private``.
For state variables, ``external`` is not possible.

``external``
    External functions are part of the contract interface,
    which means they can be called from other contracts and
    via transactions. An external function ``f`` cannot be called
    internally (i.e. ``f()`` does not work, but ``this.f()`` works).

``public``
    Public functions are part of the contract interface
    and can be either called internally or via
    messages. For public state variables, an automatic getter
    function (see below) is generated.

``internal``
    Those functions and state variables can only be
    accessed internally (i.e. from within the current contract
    or contracts deriving from it), without using ``this``.
    This is the default visibility level for state variables.

``private``
    Private functions and state variables are only
    visible for the contract they are defined in and not in
    derived contracts.

.. note::
    Everything that is inside a contract is visible to
    all observers external to the blockchain. Making something ``private``
    only prevents other contracts from reading or modifying
    the information, but it will still be visible to the
    whole world outside of the blockchain.

The visibility specifier is given after the type for
state variables and between parameter list and
return parameter list for functions.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f(uint a) private pure returns (uint b) { return a + 1; }
        function setData(uint a) internal { data = a; }
        uint public data;
    }

In the following example, ``D``, can call ``c.getData()`` to retrieve the value of
``data`` in state storage, but is not able to call ``f``. Contract ``E`` is derived from
``C`` and, thus, can call ``compute``.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        uint private data;

        function f(uint a) private pure returns(uint b) { return a + 1; }
        function setData(uint a) public { data = a; }
        function getData() public view returns(uint) { return data; }
        function compute(uint a, uint b) internal pure returns (uint) { return a + b; }
    }

    // This will not compile
    contract D {
        function readData() public {
            C c = new C();
            uint local = c.f(7); // error: member `f` is not visible
            c.setData(3);
            local = c.getData();
            local = c.compute(3, 5); // error: member `compute` is not visible
        }
    }

    contract E is C {
        function g() public {
            C c = new C();
            uint val = compute(3, 5); // access to internal member (from derived to parent contract)
        }
    }

.. index:: ! getter;function, ! function;getter
.. _getter-functions:

Getter Functions
================

The compiler automatically creates getter functions for
all **public** state variables. For the contract given below, the compiler will
generate a function called ``data`` that does not take any
arguments and returns a ``uint``, the value of the state
variable ``data``. State variables can be initialized
when they are declared.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        uint public data = 42;
    }

    contract Caller {
        C c = new C();
        function f() public view returns (uint) {
            return c.data();
        }
    }

The getter functions have external visibility. If the
symbol is accessed internally (i.e. without ``this.``),
it evaluates to a state variable.  If it is accessed externally
(i.e. with ``this.``), it evaluates to a function.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract C {
        uint public data;
        function x() public returns (uint) {
            data = 3; // internal access
            return this.data(); // external access
        }
    }

If you have a ``public`` state variable of array type, then you can only retrieve
single elements of the array via the generated getter function. This mechanism
exists to avoid high gas costs when returning an entire array. You can use
arguments to specify which individual element to return, for example
``myArray(0)``. If you want to return an entire array in one call, then you need
to write a function, for example:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract arrayExample {
        // public state variable
        uint[] public myArray;

        // Getter function generated by the compiler
        /*
        function myArray(uint i) public view returns (uint) {
            return myArray[i];
        }
        */

        // function that returns entire array
        function getArray() public view returns (uint[] memory) {
            return myArray;
        }
    }

Now you can use ``getArray()`` to retrieve the entire array, instead of
``myArray(i)``, which returns a single element per call.

The next example is more complex:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract Complex {
        struct Data {
            uint a;
            bytes3 b;
            mapping (uint => uint) map;
            uint[3] c;
            uint[] d;
            bytes e;
        }
        mapping (uint => mapping(bool => Data[])) public data;
    }

It generates a function of the following form. The mapping and arrays (with the
exception of byte arrays) in the struct are omitted because there is no good way
to select individual struct members or provide a key for the mapping:

.. code-block:: solidity

    function data(uint arg1, bool arg2, uint arg3)
        public
        returns (uint a, bytes3 b, bytes memory e)
    {
        a = data[arg1][arg2][arg3].a;
        b = data[arg1][arg2][arg3].b;
        e = data[arg1][arg2][arg3].e;
    }
