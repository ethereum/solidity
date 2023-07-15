.. index:: ! contract;interface, ! interface contract

.. _interfaces:

**********
Interfaces
**********

Interfaces are similar to abstract contracts, but they cannot have any functions implemented.
There are further restrictions:

- They cannot inherit from other contracts, but they can inherit from other interfaces.
- All declared functions must be external in the interface, even if they are public in the contract.
- They cannot declare a constructor.
- They cannot declare state variables.
- They cannot declare modifiers.

Some of these restrictions might be lifted in the future.

Interfaces are basically limited to what the Contract ABI can represent, and the conversion between the ABI and
an interface should be possible without any information loss.

Interfaces are denoted by their own keyword:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.9.0;

    interface Token {
        enum TokenType { Fungible, NonFungible }
        struct Coin { string obverse; string reverse; }
        function transfer(address recipient, uint amount) external;
    }

Contracts can inherit interfaces as they would inherit other contracts.

All functions declared in interfaces are implicitly ``virtual`` and any
functions that override them do not need the ``override`` keyword.
This does not automatically mean that an overriding function can be overridden again -
this is only possible if the overriding function is marked ``virtual``.

Interfaces can inherit from other interfaces. This has the same rules as normal
inheritance.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.9.0;

    interface ParentA {
        function test() external returns (uint256);
    }

    interface ParentB {
        function test() external returns (uint256);
    }

    interface SubInterface is ParentA, ParentB {
        // Must redefine test in order to assert that the parent
        // meanings are compatible.
        function test() external override(ParentA, ParentB) returns (uint256);
    }

Types defined inside interfaces and other contract-like structures
can be accessed from other contracts: ``Token.TokenType`` or ``Token.Coin``.

.. warning::

    Interfaces have supported ``enum`` types since :doc:`Solidity version 0.5.0 <050-breaking-changes>`, make
    sure the pragma version specifies this version as a minimum.
