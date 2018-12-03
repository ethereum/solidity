###########################
Frequently Asked Questions
###########################

This list was originally compiled by `fivedogit <mailto:fivedogit@gmail.com>`_.


***************
Basic Questions
***************

What is the transaction "payload"?
==================================

This is just the bytecode "data" sent along with the request.


Create a contract that can be killed and return funds
=====================================================

First, a word of warning: Killing contracts sounds like a good idea, because "cleaning up"
is always good, but as seen above, it does not really clean up. Furthermore,
if Ether is sent to removed contracts, the Ether will be forever lost.

If you want to deactivate your contracts, it is preferable to **disable** them by changing some
internal state which causes all functions to throw. This will make it impossible
to use the contract and ether sent to the contract will be returned automatically.

Now to answering the question: Inside a constructor, ``msg.sender`` is the
creator. Save it. Then ``selfdestruct(creator);`` to kill and return funds.

`example <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/05_greeter.sol>`_

Note that if you ``import "mortal"`` at the top of your contracts and declare
``contract SomeContract is mortal { ...`` and compile with a compiler that already
has it (which includes `Remix <https://remix.ethereum.org/>`_), then
``kill()`` is taken care of for you. Once a contract is "mortal", then you can
``contractname.kill.sendTransaction({from:eth.coinbase})``, just the same as my
examples.

If I return an ``enum``, I only get integer values in web3.js. How to get the named values?
===========================================================================================

Enums are not supported by the ABI, they are just supported by Solidity.
You have to do the mapping yourself for now, we might provide some help
later.

Can state variables be initialized in-line?
===========================================

Yes, this is possible for all types (even for structs). However, for arrays it
should be noted that you must declare them as static memory arrays.

Examples::

    pragma solidity >=0.4.0 <0.6.0;

    contract C {
        struct S {
            uint a;
            uint b;
        }

        S public x = S(1, 2);
        string name = "Ada";
        string[4] adaArr = ["This", "is", "an", "array"];
    }

    contract D {
        C c = new C();
    }

How do structs work?
====================

See `struct_and_for_loop_tester.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/65_struct_and_for_loop_tester.sol>`_.

What are some examples of basic string manipulation (``substring``, ``indexOf``, ``charAt``, etc)?
==================================================================================================

There are some string utility functions at `stringUtils.sol <https://github.com/ethereum/dapp-bin/blob/master/library/stringUtils.sol>`_
which will be extended in the future. In addition, Arachnid has written `solidity-stringutils <https://github.com/Arachnid/solidity-stringutils>`_.

For now, if you want to modify a string (even when you only want to know its length),
you should always convert it to a ``bytes`` first::

    pragma solidity >=0.4.0 <0.6.0;

    contract C {
        string s;

        function append(byte c) public {
            bytes(s).push(c);
        }

        function set(uint i, byte c) public {
            bytes(s)[i] = c;
        }
    }


Can I concatenate two strings?
==============================

Yes, you can use ``abi.encodePacked``::

    pragma solidity >=0.4.0 <0.6.0;

    library ConcatHelper {
        function concat(bytes memory a, bytes memory b)
                internal pure returns (bytes memory) {
            return abi.encodePacked(a, b);
        }
    }


Why is the low-level function ``.call()`` less favorable than instantiating a contract with a variable (``ContractB b;``) and executing its functions (``b.doSomething();``)?
=============================================================================================================================================================================

If you use actual functions, the compiler will tell you if the types
or your arguments do not match, if the function does not exist
or is not visible and it will do the packing of the
arguments for you.

See `ping.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_ping.sol>`_ and
`pong.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_pong.sol>`_.

Are comments included with deployed contracts and do they increase deployment gas?
==================================================================================

No, everything that is not needed for execution is removed during compilation.
This includes, among others, comments, variable names and type names.

What happens if you send ether along with a function call to a contract?
========================================================================

It gets added to the total balance of the contract, just like when you send ether when creating a contract.
You can only send ether along to a function that has the ``payable`` modifier,
otherwise an exception is thrown.

******************
Advanced Questions
******************

How do you get a random number in a contract? (Implement a self-returning gambling contract.)
=============================================================================================

Getting randomness right is often the crucial part in a crypto project and
most failures result from bad random number generators.

If you do not want it to be safe, you build something similar to the `coin flipper <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/35_coin_flipper.sol>`_
but otherwise, rather use a contract that supplies randomness, like the `RANDAO <https://github.com/randao/randao>`_.

Get return value from non-constant function from another contract
=================================================================

The key point is that the calling contract needs to know about the function it intends to call.

See `ping.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_ping.sol>`_
and `pong.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_pong.sol>`_.

How do you create 2-dimensional arrays?
=======================================

See `2D_array.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/55_2D_array.sol>`_.

Note that filling a 10x10 square of ``uint8`` + contract creation took more than ``800,000``
gas at the time of this writing. 17x17 took ``2,000,000`` gas. With the limit at
3.14 million... well, there’s a pretty low ceiling for what you can create right
now.

Note that merely "creating" the array is free, the costs are in filling it.

Note2: Optimizing storage access can pull the gas costs down considerably, because
32 ``uint8`` values can be stored in a single slot. The problem is that these optimizations
currently do not work across loops and also have a problem with bounds checking.
You might get much better results in the future, though.

How do I initialize a contract with only a specific amount of wei?
==================================================================

Currently the approach is a little ugly, but there is little that can be done to improve it.
In the case of a ``contract A`` calling a new instance of ``contract B``, parentheses have to be used around
``new B`` because ``B.value`` would refer to a member of ``B`` called ``value``.
You will need to make sure that you have both contracts aware of each other's presence and that ``contract B`` has a ``payable`` constructor.
In this example::

    pragma solidity >0.4.99 <0.6.0;

    contract B {
        constructor() public payable {}
    }

    contract A {
        B child;

        function test() public {
            child = (new B).value(10)(); //construct a new B with 10 wei
        }
    }

Can a contract pass an array (static size) or string or ``bytes`` (dynamic size) to another contract?
=====================================================================================================

Sure. Take care that if you cross the memory / storage boundary,
independent copies will be created::

    pragma solidity >=0.4.16 <0.6.0;

    contract C {
        uint[20] x;

        function f() public {
            g(x);
            h(x);
        }

        function g(uint[20] memory y) internal pure {
            y[2] = 3;
        }

        function h(uint[20] storage y) internal {
            y[3] = 4;
        }
    }

The call to ``g(x)`` will not have an effect on ``x`` because it needs
to create an independent copy of the storage value in memory.
On the other hand, ``h(x)`` successfully modifies ``x`` because only
a reference and not a copy is passed.

What does the following strange check do in the Custom Token contract?
======================================================================

::

    require((balanceOf[_to] + _value) >= balanceOf[_to]);

Integers in Solidity (and most other machine-related programming languages) are restricted to a certain range.
For ``uint256``, this is ``0`` up to ``2**256 - 1``. If the result of some operation on those numbers
does not fit inside this range, it is truncated. These truncations can have
`serious consequences <https://en.bitcoin.it/wiki/Value_overflow_incident>`_, so code like the one
above is necessary to avoid certain attacks.


Why are explicit conversions between fixed-size bytes types and integer types failing?
======================================================================================

Since version 0.5.0 explicit conversions between fixed-size byte arrays and integers are only allowed,
if both types have the same size. This prevents unexpected behaviour when truncating or padding.
Such conversions are still possible, but intermediate casts are required that make the desired
truncation and padding convention explicit. See :ref:`types-conversion-elementary-types` for a full
explanation and examples.


Why can number literals not be converted to fixed-size bytes types?
===================================================================

Since version 0.5.0 only hexadecimal number literals can be converted to fixed-size bytes
types and only if the number of hex digits matches the size of the type. See :ref:`types-conversion-literals`
for a full explanation and examples.



More Questions?
===============

If you have more questions or your question is not answered here, please talk to us on
`gitter <https://gitter.im/ethereum/solidity>`_ or file an `issue <https://github.com/ethereum/solidity/issues>`_.
