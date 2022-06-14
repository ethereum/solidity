.. index:: ! error, revert, ! selector; of an error
.. _errors:

*******************************
Errors and the Revert Statement
*******************************

Errors in Solidity provide a convenient and gas-efficient way to explain to the
user why an operation failed. They can be defined inside and outside of contracts (including interfaces and libraries).

They have to be used together with the :ref:`revert statement <revert-statement>`
which causes
all changes in the current call to be reverted and passes the error data back to the
caller.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;

    /// Insufficient balance for transfer. Needed `required` but only
    /// `available` available.
    /// @param available balance available.
    /// @param required requested amount to transfer.
    error InsufficientBalance(uint256 available, uint256 required);

    contract TestToken {
        mapping(address => uint) balance;
        function transfer(address to, uint256 amount) public {
            if (amount > balance[msg.sender])
                revert InsufficientBalance({
                    available: balance[msg.sender],
                    required: amount
                });
            balance[msg.sender] -= amount;
            balance[to] += amount;
        }
        // ...
    }

Errors cannot be overloaded or overridden but are inherited.
The same error can be defined in multiple places as long as the scopes are distinct.
Instances of errors can only be created using ``revert`` statements.

The error creates data that is then passed to the caller with the revert operation
to either return to the off-chain component or catch it in a :ref:`try/catch statement <try-catch>`.
Note that an error can only be caught when coming from an external call,
reverts happening in internal calls or inside the same function cannot be caught.

If you do not provide any parameters, the error only needs four bytes of
data and you can use :ref:`NatSpec <natspec>` as above
to further explain the reasons behind the error, which is not stored on chain.
This makes this a very cheap and convenient error-reporting feature at the same time.

More specifically, an error instance is ABI-encoded in the same way as
a function call to a function of the same name and types would be
and then used as the return data in the ``revert`` opcode.
This means that the data consists of a 4-byte selector followed by :ref:`ABI-encoded<abi>` data.
The selector consists of the first four bytes of the keccak256-hash of the signature of the error type.

.. note::
    It is possible for a contract to revert
    with different errors of the same name or even with errors defined in different places
    that are indistinguishable by the caller. For the outside, i.e. the ABI,
    only the name of the error is relevant, not the contract or file where it is defined.

The statement ``require(condition, "description");`` would be equivalent to
``if (!condition) revert Error("description")`` if you could define
``error Error(string)``.
Note, however, that ``Error`` is a built-in type and cannot be defined in user-supplied code.

Similarly, a failing ``assert`` or similar conditions will revert with an error
of the built-in type ``Panic(uint256)``.

.. note::
    Error data should only be used to give an indication of failure, but
    not as a means for control-flow. The reason is that the revert data
    of inner calls is propagated back through the chain of external calls
    by default. This means that an inner call
    can "forge" revert data that looks like it could have come from the
    contract that called it.

Members of Errors
=================

- ``error.selector``: A ``bytes4`` value containing the error selector.
