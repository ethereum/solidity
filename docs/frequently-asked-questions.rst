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


What is the ``memory`` keyword? What does it do?
================================================

The types where the so-called storage location is important are structs
and arrays. If you e.g. pass such variables in function calls, their
data is not copied if it can stay in memory or stay in storage.
This means that you can modify their content in the called function
and these modifications will still be visible in the caller.


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

.. warning::
    Prior to version 0.5.0, a common mistake was to declare a local variable and assume that it will
    be created in memory, although it will be created in storage. Using such a variable without initializing
    it could lead to unexpected behavior. Starting from 0.5.0, however, storage variables have to be initialized,
    which should prevent these kinds of mistakes.

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



More Questions?
===============

If you have more questions or your question is not answered here, please talk to us on
`gitter <https://gitter.im/ethereum/solidity>`_ or file an `issue <https://github.com/ethereum/solidity/issues>`_.
