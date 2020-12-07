********************************
Solidity IR-based Codegen Changes
********************************

This section highlights the main differences between the old and the IR-based codegen,
along with the reasoning behind the changes and how to update affected code.

Semantic Only Changes
=====================

This section lists the changes that are semantic-only, thus potentially
hiding new and different behavior in existing code.

 * When storage structs are deleted, every storage slot that contains a member of the struct is set to zero entirely. Formally, padding space was left untouched.
Consequently, if the padding space within a struct is used to store data (e.g. in the context of a contract upgrade), you have to be aware that ``delete`` will now also clear the added member (while it wouldn't have been cleared in the past).

::
    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >0.7.0;

    contract C {
        struct S {
            uint64 y;
            uint64 z;
        }
        S s;
        function f() public {
            // ...
            delete s;
            // s occupies only first 16 bytes of the 32 bytes slot
            // delete will write zero to the full slot
        }
    }

We have the same behavior for implicit delete, for example when array of structs is shortened.

 * The order of contract initialization has changed in case of inheritance.

The order used to be:
 - All state variables are zero-initialized at the beginning.
 - Evaluate base constructor arguments from most derived to most base contract.
 - Initialize all state variables in the whole inheritance hierarchy from most base to most derived.
 - Run the constructor, if present, for all contracts in the linearized hierarchy from most base to most derived.

New order:
 - All state variables are zero-initialized at the beginning.
 - Evaluate base constructor arguments from most derived to most base contract.
 - For every contract in order from most base to most derived in the linearized hierarchy execute:
     1. If present at declaration, initial values are assigned to state variables.
     2. Constructor, if present.

This causes differences in some contracts, for example:
::
    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >0.7.0;

    contract A {
        uint x;
        constructor() {
            x = 42;
        }
        function f() public view returns(uint256) {
            return x;
        }
    }
    contract B is A {
        uint public y = f();
    }

Previously, ``y`` would be set to 0. This is due to the fact that we would first initialize state variables: First, ``x`` is set to 0, and when initializing ``y``, ``f()`` would return 0 causing ``y`` to be 0 as well.
With the new rules, ``y`` will be set to 42. We first initialize ``x`` to 0, then call A's constructor which sets ``x`` to 42. Finally, when initializing ``y``, ``f()`` returns 42 causing ``y`` to be 42.
