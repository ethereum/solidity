###########################
Frequently Asked Questions
###########################

This list was originally compiled by `fivedogit <mailto:fivedogit@gmail.com>`_.


***************
Basic Questions
***************


If I return an ``enum``, I only get integer values in web3.js. How to get the named values?
===========================================================================================

Enums are not supported by the ABI, they are just supported by Solidity.
You have to do the mapping yourself for now, we might provide some help
later.


Why is the low-level function ``.call()`` less favorable than instantiating a contract with a variable (``ContractB b;``) and executing its functions (``b.doSomething();``)?
=============================================================================================================================================================================

If you use actual functions, the compiler will tell you if the types
or your arguments do not match, if the function does not exist
or is not visible and it will do the packing of the
arguments for you.

See `ping.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_ping.sol>`_ and
`pong.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_pong.sol>`_.



When returning a value of say ``uint`` type, is it possible to return an ``undefined`` or "null"-like value?
============================================================================================================

This is not possible, because all types use up the full value range.

You have the option to ``throw`` on error, which will also revert the whole
transaction, which might be a good idea if you ran into an unexpected
situation.

If you do not want to throw, you can return a pair::

    pragma solidity >0.4.23 <0.5.0;

    contract C {
        uint[] counters;

        function getCounter(uint index)
            public
            view
            returns (uint counter, bool error) {
                if (index >= counters.length)
                    return (0, true);
                else
                    return (counters[index], false);
        }

        function checkCounter(uint index) public view {
            (uint counter, bool error) = getCounter(index);
            if (error) {
                // ...
            } else {
                // ...
            }
        }
    }



What is the ``memory`` keyword? What does it do?
================================================

The Ethereum Virtual Machine has three areas where it can store items.

The first is "storage", where all the contract state variables reside.
Every contract has its own storage and it is persistent between function calls
and quite expensive to use.

The second is "memory", this is used to hold temporary values. It
is erased between (external) function calls and is cheaper to use.

The third one is the stack, which is used to hold small local variables.
It is almost free to use, but can only hold a limited amount of values.

For almost all types, you cannot specify where they should be stored, because
they are copied everytime they are used.

The types where the so-called storage location is important are structs
and arrays. If you e.g. pass such variables in function calls, their
data is not copied if it can stay in memory or stay in storage.
This means that you can modify their content in the called function
and these modifications will still be visible in the caller.

There are defaults for the storage location depending on which type
of variable it concerns:

* state variables are always in storage
* function arguments are in memory by default
* local variables of struct, array or mapping type reference storage by default
* local variables of value type (i.e. neither array, nor struct nor mapping) are stored in the stack

Example::

    pragma solidity ^0.4.0;

    contract C {
        uint[] data1;
        uint[] data2;

        function appendOne() public {
            append(data1);
        }

        function appendTwo() public {
            append(data2);
        }

        function append(uint[] storage d) internal {
            d.push(1);
        }
    }

The function ``append`` can work both on ``data1`` and ``data2`` and its modifications will be
stored permanently. If you remove the ``storage`` keyword, the default
is to use ``memory`` for function arguments. This has the effect that
at the point where ``append(data1)`` or ``append(data2)`` is called, an
independent copy of the state variable is created in memory and
``append`` operates on this copy (which does not support ``.push`` - but that
is another issue). The modifications to this independent copy do not
carry back to ``data1`` or ``data2``.

A common mistake is to declare a local variable and assume that it will
be created in memory, although it will be created in storage::

    /// THIS CONTRACT CONTAINS AN ERROR

    pragma solidity ^0.4.0;

    contract C {
        uint someVariable;
        uint[] data;

        function f() public {
            uint[] x;
            x.push(2);
            data = x;
        }
    }

The type of the local variable ``x`` is ``uint[] storage``, but since
storage is not dynamically allocated, it has to be assigned from
a state variable before it can be used. So no space in storage will be
allocated for ``x``, but instead it functions only as an alias for
a pre-existing variable in storage.

What will happen is that the compiler interprets ``x`` as a storage
pointer and will make it point to the storage slot ``0`` by default.
This has the effect that ``someVariable`` (which resides at storage
slot ``0``) is modified by ``x.push(2)``.

The correct way to do this is the following::

    pragma solidity ^0.4.0;

    contract C {
        uint someVariable;
        uint[] data;

        function f() public {
            uint[] x = data;
            x.push(2);
        }
    }

******************
Advanced Questions
******************



Get return value from non-constant function from another contract
=================================================================

The key point is that the calling contract needs to know about the function it intends to call.

See `ping.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_ping.sol>`_
and `pong.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_pong.sol>`_.


Can a contract pass an array (static size) or string or ``bytes`` (dynamic size) to another contract?
=====================================================================================================

Sure. Take care that if you cross the memory / storage boundary,
independent copies will be created::

    pragma solidity ^0.4.16;

    contract C {
        uint[20] x;

        function f() public {
            g(x);
            h(x);
        }

        function g(uint[20] y) internal pure {
            y[2] = 3;
        }

        function h(uint[20] storage y) internal {
            y[3] = 4;
        }
    }

The call to ``g(x)`` will not have an effect on ``x`` because it needs
to create an independent copy of the storage value in memory
(the default storage location is memory). On the other hand,
``h(x)`` successfully modifies ``x`` because only a reference
and not a copy is passed.


What could have happened if an account has storage value(s) but no code?  Example: http://test.ether.camp/account/5f740b3a43fbb99724ce93a879805f4dc89178b5
==========================================================================================================================================================

The last thing a constructor does is returning the code of the contract.
The gas costs for this depend on the length of the code and it might be
that the supplied gas is not enough. This situation is the only one
where an "out of gas" exception does not revert changes to the state,
i.e. in this case the initialisation of the state variables.

https://github.com/ethereum/wiki/wiki/Subtleties

After a successful CREATE operation's sub-execution, if the operation returns x, 5 * len(x) gas is subtracted from the remaining gas before the contract is created. If the remaining gas is less than 5 * len(x), then no gas is subtracted, the code of the created contract becomes the empty string, but this is not treated as an exceptional condition - no reverts happen.



More Questions?
===============

If you have more questions or your question is not answered here, please talk to us on
`gitter <https://gitter.im/ethereum/solidity>`_ or file an `issue <https://github.com/ethereum/solidity/issues>`_.
