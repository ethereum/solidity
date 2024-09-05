.. index:: ! transient storage, ! transient, tstore, tload

.. _transient-storage:

*****************
Transient Storage
*****************

Transient storage is another data location besides memory, storage, calldata
(and return-data and code) which was introduced alongside its respective opcodes
``TSTORE`` and ``TLOAD`` by `EIP-1153 <https://eips.ethereum.org/EIPS/eip-1153>`_.
This new data location behaves as a key-value store similar to storage with the main
difference being that data in transient storage is not permanent, but is scoped to
the current transaction only, after which it will be reset to zero.
Since the content of transient storage has very limited lifetime and size,
it does not need to be stored permanently as a part of state
and the associated gas costs are much lower than in case of storage.
EVM version ``cancun`` or newer is required for transient storage to be available.

Transient storage variables cannot be initialized in place, i.e., they cannot be assigned
to upon declaration, since the value would be cleared at the end of the creation transaction,
rendering the initialization ineffective.
Transient variables will be :ref:`default value<default-value>` initialized depending on
their underlying type.
``constant`` and ``immutable`` variables conflict with transient storage, since
their values are either inlined or directly stored in code.

Transient storage variables have completely independent address space from storage,
so that the order of transient state variables does not affect the layout of storage
state variables and vice-versa.
They do need distinct names though because all state variables share the same namespace.
It is also important to note that the values in transient storage are packed in the
same fashion as those in persistent storage.
See :ref:`Storage Layout <storage-inplace-encoding>` for more information.

Besides that, transient variables can have visibility as well and ``public`` ones will
have a getter function generated automatically as usual.

Note that, currently, such use of ``transient`` as a data location is only allowed for
:ref:`value type <value-types>` state variable declarations.
Reference types, such as arrays, mappings and structs, as well as local or parameter
variables are not yet supported.

An expected canonical use case for transient storage is cheaper reentrancy locks,
which can be readily implemented with the opcodes as showcased next.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.28;

    contract Generosity {
        mapping(address => bool) sentGifts;
        bool transient locked;

        modifier nonReentrant {
            require(!locked, "Reentrancy attempt");
            locked = true;
            _;
            // Unlocks the guard, making the pattern composable.
            // After the function exits, it can be called again, even in the same transaction.
            locked = false;
        }

        function claimGift() nonReentrant public {
            require(address(this).balance >= 1 ether);
            require(!sentGifts[msg.sender]);
            (bool success, ) = msg.sender.call{value: 1 ether}("");
            require(success);

            // In a reentrant function, doing this last would open up the vulnerability
            sentGifts[msg.sender] = true;
        }
    }

Transient storage is private to the contract that owns it, in the same way as persistent storage.
Only owning contract frames may access their transient storage, and when they do, all the frames access the same transient store.

Transient storage is part of the EVM state and is subject to the same mutability enforcements
as persistent storage. As such, any read access to it is not ``pure`` and writing access is not ``view``.

If the ``TSTORE`` opcode is called within the context of a ``STATICCALL``,
it will result in an exception instead of performing the modification.
``TLOAD`` is allowed within the context of a ``STATICCALL``.

When transient storage is used in the context of ``DELEGATECALL`` or ``CALLCODE``,
then the owning contract of the transient storage is the contract that issued ``DELEGATECALL``
or ``CALLCODE`` instruction (the caller) as with persistent storage.
When transient storage is used in the context of ``CALL`` or ``STATICCALL``,
then the owning contract of the transient storage is the contract that is the target
of the ``CALL`` or ``STATICCALL`` instruction (the callee).

.. note::
    In the case of ``DELEGATECALL``, since references to transient storage variables
    are currently not supported, it is not possible to pass those into library calls.
    In libraries, access to transient storage is only possible using inline assembly.

If a frame reverts, all writes to transient storage that took place between entry
to the frame and the return are reverted, including those that took place in inner calls.
The caller of an external call may employ a ``try ... catch`` block to prevent reverts
bubbling up from the inner calls.

*********************************************************************
Composability of Smart Contracts and the Caveats of Transient Storage
*********************************************************************

Given the caveats mentioned in the specification of EIP-1153,
in order to preserve the composability of your smart contract,
utmost care is recommended for more advanced use cases of transient storage.

For smart contracts, composability is a very important design principle to achieve self-contained behaviour,
such that multiple calls into individual smart contracts can be composed to more complex applications.
So far the EVM largely guaranteed composable behaviour, since multiple calls into a smart contract
within a complex transaction are virtually indistinguishable from multiple calls to the contract
stretched over several transactions. However, transient storage allows a violation of this principle,
and incorrect use may lead to complex bugs that only surface when used across several calls.

Let's illustrate the problem with a simple example:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.28;

    contract MulService {
        uint transient multiplier;
        function setMultiplier(uint mul) external {
            multiplier = mul;
        }

        function multiply(uint value) external view returns (uint) {
            return value * multiplier;
        }
    }

and  a sequence of external calls:

.. code-block:: solidity

    setMultiplier(42);
    multiply(1);
    multiply(2);

If the example used memory or storage to store the multiplier, it would be fully composable.
It would not matter whether you split the sequence into separate transactions or grouped them in some way.
You would always get the same result: after ``multiplier`` is set to ``42``, the subsequent calls
would return ``42`` and ``84`` respectively.
This enables use cases such as batching calls from multiple transactions
together to reduce gas costs.
Transient storage potentially breaks such use cases since composability can no longer be taken for granted.
In the example, if the calls are not executed in the same transaction, then ``multiplier``
is reset and the next calls to function ``multiply`` would always return ``0``.

As another example, since transient storage is constructed as a relatively cheap key-value store,
a smart contract author may be tempted to use transient storage as a replacement for in-memory mappings
without keeping track of the modified keys in the mapping and thereby without clearing the mapping
at the end of the call.
This, however, can easily lead to unexpected behaviour in complex transactions,
in which values set by a previous call into the contract within the same transaction remain.

The use of transient storage for reentrancy locks that are cleared at the end of the call frame
into the contract, is safe.
However, be sure to resist the temptation to save the 100 gas used for resetting the
reentrancy lock, since failing to do so, will restrict your contract to only one call
within a transaction, preventing its use in complex composed transactions,
which have been a cornerstone for complex applications on chain.

It is recommend to generally always clear transient storage completely at the end of a call
into your smart contract to avoid these kinds of issues and to simplify
the analysis of the behaviour of your contract within complex transactions.
Check the `Security Considerations section of EIP-1153 <https://eips.ethereum.org/EIPS/eip-1153#security-considerations>`_
for further details.