.. index:: ! functions, ! function;free

.. _functions:

*********
Functions
*********

Functions can be defined inside and outside of contracts.

Functions outside of a contract, also called "free functions", always have implicit ``internal``
:ref:`visibility<visibility-and-getters>`. Their code is included in all contracts
that call them, similar to internal library functions.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.1 <0.9.0;

    function sum(uint[] memory arr) pure returns (uint s) {
        for (uint i = 0; i < arr.length; i++)
            s += arr[i];
    }

    contract ArrayExample {
        bool found;
        function f(uint[] memory arr) public {
            // This calls the free function internally.
            // The compiler will add its code to the contract.
            uint s = sum(arr);
            require(s >= 10);
            found = true;
        }
    }

.. note::
    Functions defined outside a contract are still always executed
    in the context of a contract.
    They still can call other contracts, send them Ether and destroy the contract that called them,
    among other things. The main difference to functions defined inside a contract
    is that free functions do not have direct access to the variable ``this``, storage variables and functions
    not in their scope.

.. _function-parameters-return-variables:

Function Parameters and Return Variables
========================================

Functions take typed parameters as input and may, unlike in many other
languages, also return an arbitrary number of values as output.

Function Parameters
-------------------

Function parameters are declared the same way as variables, and the name of
unused parameters can be omitted.

For example, if you want your contract to accept one kind of external call
with two integers, you would use something like the following:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract Simple {
        uint sum;
        function taker(uint a, uint b) public {
            sum = a + b;
        }
    }

Function parameters can be used as any other local variable and they can also be assigned to.

.. index:: return array, return string, array, string, array of strings, dynamic array, variably sized array, return struct, struct

Return Variables
----------------

Function return variables are declared with the same syntax after the
``returns`` keyword.

For example, suppose you want to return two results: the sum and the product of
two integers passed as function parameters, then you use something like:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract Simple {
        function arithmetic(uint a, uint b)
            public
            pure
            returns (uint sum, uint product)
        {
            sum = a + b;
            product = a * b;
        }
    }

The names of return variables can be omitted.
Return variables can be used as any other local variable and they
are initialized with their :ref:`default value <default-value>` and have that
value until they are (re-)assigned.

You can either explicitly assign to return variables and
then leave the function as above,
or you can provide return values
(either a single or :ref:`multiple ones<multi-return>`) directly with the ``return``
statement:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract Simple {
        function arithmetic(uint a, uint b)
            public
            pure
            returns (uint sum, uint product)
        {
            return (a + b, a * b);
        }
    }

If you use an early ``return`` to leave a function that has return variables,
you must provide return values together with the return statement.

.. note::
    You cannot return some types from non-internal functions.
    This includes the types listed below and any composite types that recursively contain them:

    - mappings,
    - internal function types,
    - reference types with location set to ``storage``,
    - multi-dimensional arrays (applies only to :ref:`ABI coder v1 <abi_coder>`),
    - structs (applies only to :ref:`ABI coder v1 <abi_coder>`).

    This restriction does not apply to library functions because of their different :ref:`internal ABI <library-selectors>`.

.. _multi-return:

Returning Multiple Values
-------------------------

When a function has multiple return types, the statement ``return (v0, v1, ..., vn)`` can be used to return multiple values.
The number of components must be the same as the number of return variables
and their types have to match, potentially after an :ref:`implicit conversion <types-conversion-elementary-types>`.

.. _state-mutability:

State Mutability
================

.. index:: ! view function, function;view

.. _view-functions:

View Functions
--------------

Functions can be declared ``view`` in which case they promise not to modify the state.

.. note::
  If the compiler's EVM target is Byzantium or newer (default) the opcode
  ``STATICCALL`` is used when ``view`` functions are called, which enforces the state
  to stay unmodified as part of the EVM execution. For library ``view`` functions
  ``DELEGATECALL`` is used, because there is no combined ``DELEGATECALL`` and ``STATICCALL``.
  This means library ``view`` functions do not have run-time checks that prevent state
  modifications. This should not impact security negatively because library code is
  usually known at compile-time and the static checker performs compile-time checks.

The following statements are considered modifying the state:

#. Writing to state variables.
#. :ref:`Emitting events <events>`.
#. :ref:`Creating other contracts <creating-contracts>`.
#. Using ``selfdestruct``.
#. Sending Ether via calls.
#. Calling any function not marked ``view`` or ``pure``.
#. Using low-level calls.
#. Using inline assembly that contains certain opcodes.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        function f(uint a, uint b) public view returns (uint) {
            return a * (b + 42) + block.timestamp;
        }
    }

.. note::
  ``constant`` on functions used to be an alias to ``view``, but this was dropped in version 0.5.0.

.. note::
  Getter methods are automatically marked ``view``.

.. note::
  Prior to version 0.5.0, the compiler did not use the ``STATICCALL`` opcode
  for ``view`` functions.
  This enabled state modifications in ``view`` functions through the use of
  invalid explicit type conversions.
  By using  ``STATICCALL`` for ``view`` functions, modifications to the
  state are prevented on the level of the EVM.

.. index:: ! pure function, function;pure

.. _pure-functions:

Pure Functions
--------------

Functions can be declared ``pure`` in which case they promise not to read from or modify the state.
In particular, it should be possible to evaluate a ``pure`` function at compile-time given
only its inputs and ``msg.data``, but without any knowledge of the current blockchain state.
This means that reading from ``immutable`` variables can be a non-pure operation.

.. note::
  If the compiler's EVM target is Byzantium or newer (default) the opcode ``STATICCALL`` is used,
  which does not guarantee that the state is not read, but at least that it is not modified.

In addition to the list of state modifying statements explained above, the following are considered reading from the state:

#. Reading from state variables.
#. Accessing ``address(this).balance`` or ``<address>.balance``.
#. Accessing any of the members of ``block``, ``tx``, ``msg`` (with the exception of ``msg.sig`` and ``msg.data``).
#. Calling any function not marked ``pure``.
#. Using inline assembly that contains certain opcodes.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        function f(uint a, uint b) public pure returns (uint) {
            return a * (b + 42);
        }
    }

Pure functions are able to use the ``revert()`` and ``require()`` functions to revert
potential state changes when an :ref:`error occurs <assert-and-require>`.

Reverting a state change is not considered a "state modification", as only changes to the
state made previously in code that did not have the ``view`` or ``pure`` restriction
are reverted and that code has the option to catch the ``revert`` and not pass it on.

This behavior is also in line with the ``STATICCALL`` opcode.

.. warning::
  It is not possible to prevent functions from reading the state at the level
  of the EVM, it is only possible to prevent them from writing to the state
  (i.e. only ``view`` can be enforced at the EVM level, ``pure`` can not).

.. note::
  Prior to version 0.5.0, the compiler did not use the ``STATICCALL`` opcode
  for ``pure`` functions.
  This enabled state modifications in ``pure`` functions through the use of
  invalid explicit type conversions.
  By using  ``STATICCALL`` for ``pure`` functions, modifications to the
  state are prevented on the level of the EVM.

.. note::
  Prior to version 0.4.17 the compiler did not enforce that ``pure`` is not reading the state.
  It is a compile-time type check, which can be circumvented doing invalid explicit conversions
  between contract types, because the compiler can verify that the type of the contract does
  not do state-changing operations, but it cannot check that the contract that will be called
  at runtime is actually of that type.

.. _special-functions:

Special Functions
=================

.. index:: ! receive ether function, function;receive, ! receive

.. _receive-ether-function:

Receive Ether Function
----------------------

A contract can have at most one ``receive`` function, declared using
``receive() external payable { ... }``
(without the ``function`` keyword).
This function cannot have arguments, cannot return anything and must have
``external`` visibility and ``payable`` state mutability.
It can be virtual, can override and can have modifiers.

The receive function is executed on a
call to the contract with empty calldata. This is the function that is executed
on plain Ether transfers (e.g. via ``.send()`` or ``.transfer()``). If no such
function exists, but a payable :ref:`fallback function <fallback-function>`
exists, the fallback function will be called on a plain Ether transfer. If
neither a receive Ether nor a payable fallback function is present, the
contract cannot receive Ether through a transaction that does not represent a payable function call and throws an
exception.

In the worst case, the ``receive`` function can only rely on 2300 gas being
available (for example when ``send`` or ``transfer`` is used), leaving little
room to perform other operations except basic logging. The following operations
will consume more gas than the 2300 gas stipend:

- Writing to storage
- Creating a contract
- Calling an external function which consumes a large amount of gas
- Sending Ether

.. warning::
    When Ether is sent directly to a contract (without a function call, i.e. sender uses ``send`` or ``transfer``)
    but the receiving contract does not define a receive Ether function or a payable fallback function,
    an exception will be thrown, sending back the Ether (this was different
    before Solidity v0.4.0). If you want your contract to receive Ether,
    you have to implement a receive Ether function (using payable fallback functions for receiving Ether is
    not recommended, since the fallback is invoked and would not fail for interface confusions
    on the part of the sender).


.. warning::
    A contract without a receive Ether function can receive Ether as a
    recipient of a *coinbase transaction* (aka *miner block reward*)
    or as a destination of a ``selfdestruct``.

    A contract cannot react to such Ether transfers and thus also
    cannot reject them. This is a design choice of the EVM and
    Solidity cannot work around it.

    It also means that ``address(this).balance`` can be higher
    than the sum of some manual accounting implemented in a
    contract (i.e. having a counter updated in the receive Ether function).

Below you can see an example of a Sink contract that uses function ``receive``.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    // This contract keeps all Ether sent to it with no way
    // to get it back.
    contract Sink {
        event Received(address, uint);
        receive() external payable {
            emit Received(msg.sender, msg.value);
        }
    }

.. index:: ! fallback function, function;fallback

.. _fallback-function:

Fallback Function
-----------------

A contract can have at most one ``fallback`` function, declared using either ``fallback () external [payable]``
or ``fallback (bytes calldata input) external [payable] returns (bytes memory output)``
(both without the ``function`` keyword).
This function must have ``external`` visibility. A fallback function can be virtual, can override
and can have modifiers.

The fallback function is executed on a call to the contract if none of the other
functions match the given function signature, or if no data was supplied at
all and there is no :ref:`receive Ether function <receive-ether-function>`.
The fallback function always receives data, but in order to also receive Ether
it must be marked ``payable``.

If the version with parameters is used, ``input`` will contain the full data sent to the contract
(equal to ``msg.data``) and can return data in ``output``. The returned data will not be
ABI-encoded. Instead it will be returned without modifications (not even padding).

In the worst case, if a payable fallback function is also used in
place of a receive function, it can only rely on 2300 gas being
available (see :ref:`receive Ether function <receive-ether-function>`
for a brief description of the implications of this).

Like any function, the fallback function can execute complex
operations as long as there is enough gas passed on to it.

.. warning::
    A ``payable`` fallback function is also executed for
    plain Ether transfers, if no :ref:`receive Ether function <receive-ether-function>`
    is present. It is recommended to always define a receive Ether
    function as well, if you define a payable fallback function
    to distinguish Ether transfers from interface confusions.

.. note::
    If you want to decode the input data, you can check the first four bytes
    for the function selector and then
    you can use ``abi.decode`` together with the array slice syntax to
    decode ABI-encoded data:
    ``(c, d) = abi.decode(input[4:], (uint256, uint256));``
    Note that this should only be used as a last resort and
    proper functions should be used instead.


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.9.0;

    contract Test {
        uint x;
        // This function is called for all messages sent to
        // this contract (there is no other function).
        // Sending Ether to this contract will cause an exception,
        // because the fallback function does not have the `payable`
        // modifier.
        fallback() external { x = 1; }
    }

    contract TestPayable {
        uint x;
        uint y;
        // This function is called for all messages sent to
        // this contract, except plain Ether transfers
        // (there is no other function except the receive function).
        // Any call with non-empty calldata to this contract will execute
        // the fallback function (even if Ether is sent along with the call).
        fallback() external payable { x = 1; y = msg.value; }

        // This function is called for plain Ether transfers, i.e.
        // for every call with empty calldata.
        receive() external payable { x = 2; y = msg.value; }
    }

    contract Caller {
        function callTest(Test test) public returns (bool) {
            (bool success,) = address(test).call(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // results in test.x becoming == 1.

            // address(test) will not allow to call ``send`` directly, since ``test`` has no payable
            // fallback function.
            // It has to be converted to the ``address payable`` type to even allow calling ``send`` on it.
            address payable testPayable = payable(address(test));

            // If someone sends Ether to that contract,
            // the transfer will fail, i.e. this returns false here.
            return testPayable.send(2 ether);
        }

        function callTestPayable(TestPayable test) public returns (bool) {
            (bool success,) = address(test).call(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // results in test.x becoming == 1 and test.y becoming 0.
            (success,) = address(test).call{value: 1}(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // results in test.x becoming == 1 and test.y becoming 1.

            // If someone sends Ether to that contract, the receive function in TestPayable will be called.
            // Since that function writes to storage, it takes more gas than is available with a
            // simple ``send`` or ``transfer``. Because of that, we have to use a low-level call.
            (success,) = address(test).call{value: 2 ether}("");
            require(success);
            // results in test.x becoming == 2 and test.y becoming 2 ether.

            return true;
        }
    }

.. index:: ! overload

.. _overload-function:

Function Overloading
====================

A contract can have multiple functions of the same name but with different parameter
types.
This process is called "overloading" and also applies to inherited functions.
The following example shows overloading of the function
``f`` in the scope of contract ``A``.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract A {
        function f(uint value) public pure returns (uint out) {
            out = value;
        }

        function f(uint value, bool really) public pure returns (uint out) {
            if (really)
                out = value;
        }
    }

Overloaded functions are also present in the external interface. It is an error if two
externally visible functions differ by their Solidity types but not by their external types.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    // This will not compile
    contract A {
        function f(B value) public pure returns (B out) {
            out = value;
        }

        function f(address value) public pure returns (address out) {
            out = value;
        }
    }

    contract B {
    }


Both ``f`` function overloads above end up accepting the address type for the ABI although
they are considered different inside Solidity.

Overload resolution and Argument matching
-----------------------------------------

Overloaded functions are selected by matching the function declarations in the current scope
to the arguments supplied in the function call. Functions are selected as overload candidates
if all arguments can be implicitly converted to the expected types. If there is not exactly one
candidate, resolution fails.

.. note::
    Return parameters are not taken into account for overload resolution.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract A {
        function f(uint8 val) public pure returns (uint8 out) {
            out = val;
        }

        function f(uint256 val) public pure returns (uint256 out) {
            out = val;
        }
    }

Calling ``f(50)`` would create a type error since ``50`` can be implicitly converted both to ``uint8``
and ``uint256`` types. On another hand ``f(256)`` would resolve to ``f(uint256)`` overload as ``256`` cannot be implicitly
converted to ``uint8``.
