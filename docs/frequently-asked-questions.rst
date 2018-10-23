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

Can you return an array or a ``string`` from a solidity function call?
======================================================================

Yes. See `array_receiver_and_returner.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/60_array_receiver_and_returner.sol>`_.

Is it possible to in-line initialize an array like so: ``string[] myarray = ["a", "b"];``
=========================================================================================

Yes. However it should be noted that this currently only works with statically sized memory arrays. You can even create an inline memory
array in the return statement.

Example::

    pragma solidity >=0.4.16 <0.6.0;

    contract C {
        function f() public pure returns (uint8[5] memory) {
            string[4] memory adaArr = ["This", "is", "an", "array"];
            adaArr[0] = "That";
            return [1, 2, 3, 4, 5];
        }
    }

Can a contract function return a ``struct``?
============================================

Yes, but only in ``internal`` function calls or if ``pragma experimental "ABIEncoderV2";`` is used.

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

How do for loops work?
======================

Very similar to JavaScript. Such as the following example:

``for (uint i = 0; i < a.length; i ++) { a[i] = i; }``

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

When returning a value of say ``uint`` type, is it possible to return an ``undefined`` or "null"-like value?
============================================================================================================

This is not possible, because all types use up the full value range.

You have the option to ``throw`` on error, which will also revert the whole
transaction, which might be a good idea if you ran into an unexpected
situation.

If you do not want to throw, you can return a pair::

    pragma solidity >0.4.23 <0.6.0;

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
                // Handle the error
            } else {
                // Do something with counter.
                require(counter > 7, "Invalid counter value");
            }
        }
    }


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

What happens to a ``struct``'s mapping when copying over a ``struct``?
======================================================================

This is a very interesting question. Suppose that we have a contract field set up like such::

    struct User {
        mapping(string => string) comments;
    }

    function somefunction public {
       User user1;
       user1.comments["Hello"] = "World";
       User user2 = user1;
    }

In this case, the mapping of the struct being copied over into ``user2`` is ignored as there is no "list of mapped keys".
Therefore it is not possible to find out which values should be copied over.

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

Can a contract function accept a two-dimensional array?
=======================================================

If you want to pass two-dimensional arrays across non-internal functions,
you most likely need to use ``pragma experimental "ABIEncoderV2";``.

What is the relationship between ``bytes32`` and ``string``? Why is it that ``bytes32 somevar = "stringliteral";`` works and what does the saved 32-byte hex value mean?
========================================================================================================================================================================

The type ``bytes32`` can hold 32 (raw) bytes. In the assignment ``bytes32 samevar = "stringliteral";``,
the string literal is interpreted in its raw byte form and if you inspect ``somevar`` and
see a 32-byte hex value, this is just ``"stringliteral"`` in hex.

The type ``bytes`` is similar, only that it can change its length.

Finally, ``string`` is basically identical to ``bytes`` only that it is assumed
to hold the UTF-8 encoding of a real string. Since ``string`` stores the
data in UTF-8 encoding it is quite expensive to compute the number of
characters in the string (the encoding of some characters takes more
than a single byte). Because of that, ``string s; s.length`` is not yet
supported and not even index access ``s[2]``. But if you want to access
the low-level byte encoding of the string, you can use
``bytes(s).length`` and ``bytes(s)[2]`` which will result in the number
of bytes in the UTF-8 encoding of the string (not the number of
characters) and the second byte (not character) of the UTF-8 encoded
string, respectively.


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

Sometimes, when I try to change the length of an array with ex: ``arrayname.length = 7;`` I get a compiler error ``Value must be an lvalue``. Why?
==================================================================================================================================================

You can resize a dynamic array in storage (i.e. an array declared at the
contract level) with ``arrayname.length = <some new length>;``. If you get the
"lvalue" error, you are probably doing one of two things wrong.

1. You might be trying to resize an array in "memory", or

2. You might be trying to resize a non-dynamic array.

::

    pragma solidity >=0.4.18 <0.6.0;

    // This will not compile
    contract C {
        int8[] dynamicStorageArray;
        int8[5] fixedStorageArray;

        function f() public {
            int8[] memory memArr;        // Case 1
            memArr.length++;             // illegal

            int8[5] storage storageArr = fixedStorageArray;   // Case 2
            storageArr.length++;                             // illegal

            int8[] storage storageArr2 = dynamicStorageArray;
            storageArr2.length++;                     // legal


        }
    }

**Important note:** In Solidity, array dimensions are declared backwards from the way you
might be used to declaring them in C or Java, but they are access as in
C or Java.

For example, ``int8[][5] somearray;`` are 5 dynamic ``int8`` arrays.

The reason for this is that ``T[5]`` is always an array of 5 ``T``'s,
no matter whether ``T`` itself is an array or not (this is not the
case in C or Java).

Is it possible to return an array of strings (``string[]``) from a Solidity function?
=====================================================================================

Only when ``pragma experimental "ABIEncoderV2";`` is used.

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
