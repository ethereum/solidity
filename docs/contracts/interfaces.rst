.. index:: ! contract;interface, ! interface contract

.. _interfaces:

**********
Interfaces
**********

Interfaces are similar to abstract contracts, but they cannot have any functions implemented. There are further restrictions:

- They cannot inherit other contracts or interfaces.
- All declared functions must be external.
- They cannot declare a constructor.
- They cannot declare state variables.

Some of these restrictions might be lifted in the future.

Interfaces are basically limited to what the Contract ABI can represent, and the conversion between the ABI and
an interface should be possible without any information loss.

Interfaces are denoted by their own keyword:

::

    pragma solidity >=0.5.0 <0.7.0;

    interface Token {
        enum TokenType { Fungible, NonFungible }
        struct Coin { string obverse; string reverse; }
        function transfer(address recipient, uint amount) external;
    }

Contracts can inherit interfaces as they would inherit other contracts.

Types defined inside interfaces and other contract-like structures
can be accessed from other contracts: ``Token.TokenType`` or ``Token.Coin``.
