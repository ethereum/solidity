.. index:: contract, state variable, function, event, struct, enum, function;modifier

.. _contract_structure:

***********************
Structure of a Contract
***********************

Contracts in Solidity are similar to classes in object-oriented languages.
Each contract can contain declarations of :ref:`structure-state-variables`, :ref:`structure-functions`,
:ref:`structure-function-modifiers`, :ref:`structure-events`, :ref:`structure-struct-types` and :ref:`structure-enum-types`.
Furthermore, contracts can inherit from other contracts.

.. _structure-state-variables:

State Variables
===============

State variables are values which are permanently stored in contract storage.

::

    pragma solidity ^0.4.0;

    contract SimpleStorage {
        uint storedData; // State variable
        // ...
    }

See the :ref:`types` section for valid state variable types and
:ref:`visibility-and-getters` for possible choices for
visibility.

.. _structure-functions:

Functions
=========

Functions are the executable units of code within a contract.

::

    pragma solidity ^0.4.0;

    contract SimpleAuction {
        function bid() public payable { // Function
            // ...
        }
    }

:ref:`function-calls` can happen internally or externally
and have different levels of visibility (:ref:`visibility-and-getters`)
towards other contracts.

.. _structure-function-modifiers:

Function Modifiers
==================

Function modifiers can be used to amend the semantics of functions in a declarative way
(see :ref:`modifiers` in contracts section).

::

    pragma solidity ^0.4.22;

    contract Purchase {
        address public seller;

        modifier onlySeller() { // Modifier
            require(
                msg.sender == seller,
                "Only seller can call this."
            );
            _;
        }

        function abort() public onlySeller { // Modifier usage
            // ...
        }
    }

.. _structure-events:

Events
======

Events are convenience interfaces with the EVM logging facilities.

::

    pragma solidity ^0.4.21;

    contract SimpleAuction {
        event HighestBidIncreased(address bidder, uint amount); // Event

        function bid() public payable {
            // ...
            emit HighestBidIncreased(msg.sender, msg.value); // Triggering event
        }
    }

See :ref:`events` in contracts section for information on how events are declared
and can be used from within a dapp.

.. _structure-struct-types:

Struct Types
=============

Structs are custom defined types that can group several variables (see
:ref:`structs` in types section).

::

    pragma solidity ^0.4.0;

    contract Ballot {
        struct Voter { // Struct
            uint weight;
            bool voted;
            address delegate;
            uint vote;
        }
    }

.. _structure-enum-types:

Enum Types
==========

Enums can be used to create custom types with a finite set of 'constant values' (see
:ref:`enums` in types section).

::

    pragma solidity ^0.4.0;

    contract Purchase {
        enum State { Created, Locked, Inactive } // Enum
    }


.. _structure-modifier-areas:

Modifier Areas
==============

Modifier areas can be used to apply modifier invocations,
a visibility specifier, or a mutability specifier to an
entire group of functions. They take the following syntax:

::

    pragma soliditiy [TBD];

    contract Purchase {
        private State currentState = Created;

        enum State { Created, Locked, Inactive } // Enum

        modifier inState(State requiredState) { require(currentState = requiredState); }

        using modifier inState(State.Created) {
            function lock() { currentState = State.Locked; }

            // Other functions only callable in state Created
        }

        using modifier inState(State.Locked) {
            function inactivate() { currentState = State.Inactive; }

            // Other functions only callable in state Locked
        }

        using modifier inState(State.Inactive) {
            // Functions only callable in state Inactive
        }
    }

Multiple modifier invocations can be used in a single modifier area:
`using modifier A, B { /* ... */ }` declares a modifier area
where both modifiers `A` and `B` are used on every function.

Modifier areas can be nested by declaring a modifier area
inside the scope of another. Inside the nested modifier area,
all modifiers from all parents apply in addition to those
declared on that modifier area.

::

    contract C {
        modifier A { /* ... */ }
        modifier B { /* ... */ }

        using modifier A {
            // Functions where only A applies

            using modifier B {
                // Functions where A and B apply
            }
        }
    }

Modifier areas can also apply a mutability or visiblity specifier to
a group of functions.

::

    contract C {
        using modifier public {
            // Functions that are public
        }
        
        using modifier payable {
            // Functions that are payable
        }
    }

State and mutability specifiers also nest, just like modifiers.

::

    contract C {
        using modifier public {
            // Functions that are public
            
            using modifier payable {
                // Functions that are public and payable
            }
        }
    }

It is **not** permissible to do any of the following:
1. Declare a nested modifier area with a different visibility or
mutability than any of its parents
2. Declare a function within a modifier area with a different
visibility or mutability than any of its parents.
