.. index:: ! custom storage layout, ! storage layout specifier, ! layout at, ! base slot

.. _custom-storage-layout:

*********************
Custom Storage Layout
*********************

A contract can define an arbitrary location for its storage using the ``layout`` specifier.
The contract's state variables, including those inherited from base contracts,
start from the specified base slot instead of the default slot zero.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.29 <0.10.0;

    contract C layout at 0xAAAA + 0x11 {
        uint[3] x; // Occupies slots 0xAABB..0xAABD
    }

As the above example shows, the specifier uses the ``layout at <base-slot-expression>`` syntax
and is located in the header of a contract definition.

The layout specifier can be placed either before or after the inheritance specifier, and can appear at most once.
The ``base-slot-expression`` must be an :ref:`integer literal<rational_literals>` expression
that can be evaluated at compilation time and yields a value in the range of ``uint256``.
The use of constants initialized using such expressions and
the :ref:`built-in function erc7201<mathematical-and-cryptographic-functions>` is also allowed.

A custom layout cannot make contract's storage "wrap around".
If the selected base slot would push the static variables past the end of storage,
the compiler will issue an error.
Note that the data areas of dynamic arrays and mappings are not affected by this check because
their layout is not linear.
Regardless of the base slot used, their locations are calculated in a way that always puts them
within the range of ``uint256`` and their sizes are not known at compilation time.

While there are no other limits placed on the base slot, it is recommended to avoid locations that are
too close to the end of the address space.
Leaving too little space may complicate contract upgrades or cause problems for contracts that store
additional values past their allocated space using inline assembly.

The storage layout can only be specified for the topmost contract of an inheritance tree, and
affects locations of all the storage variables in all the contracts in that tree.
Variables are laid out according to the order of their definitions and the
positions of their contracts in the :ref:`linearized inheritance hierarchy<multi-inheritance>`
and a custom base slot preserves their relative positions, shifting them all by the same amount.

The storage layout cannot be specified for abstract contracts, interfaces and libraries.
Also, it is important to note that it does *not* affect transient state variables.

For details about storage layout and the effect of the layout specifier on it see
:ref:`layout of storage variables<storage-inplace-encoding>`.

.. warning::
    The identifiers ``layout`` and ``at`` are not yet reserved as keywords in the language.
    It is strongly recommended to avoid using them since they will become reserved in a future
    breaking release.
