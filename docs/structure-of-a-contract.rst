.. index:: contract, state variable, function, event, struct, enum, function;modifier

.. _contract_structure:

***********************
Structure of a Contract
***********************

Contracts in Solidity are similar to classes in object-oriented languages.
Each contract can contain declarations of :ref:`structure-state-variables`, :ref:`structure-functions`,
:ref:`structure-function-modifiers`, :ref:`structure-events`, :ref:`structure-struct-types` and :ref:`structure-enum-types`.
Furthermore, contracts can inherit from other contracts.

There are also special kinds of contracts called :ref:`libraries<libraries>` and :ref:`interfaces<interfaces>`.

The section about :ref:`contracts<contracts>` contains more details than this section,
which serves to provide a quick overview.

.. _structure-state-variables:

State Variables
===============

State variables are variables whose values are permanently stored in contract
storage.

::

    pragma solidity >=0.4.0 <0.7.0;

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

    pragma solidity >=0.4.0 <0.7.0;

    contract SimpleAuction {
        function bid() public payable { // Function
            // ...
        }
    }

:ref:`function-calls` can happen internally or externally
and have different levels of :ref:`visibility<visibility-and-getters>`
towards other contracts. :ref:`Functions<functions>` accept :ref:`parameters and return variables<function-parameters-return-variables>` to pass parameters
and values between them.

.. _structure-function-modifiers:

Function Modifiers
==================

Function modifiers can be used to amend the semantics of functions in a declarative way
(see :ref:`modifiers` in the contracts section).

::

    pragma solidity >=0.4.22 <0.7.0;

    contract Purchase {
        address public seller;

        modifier onlySeller() { // Modifier
            require(
                msg.sender == seller,
                "Only seller can call this."
            );
            _;
        }

        function abort() public view onlySeller { // Modifier usage
            // ...
        }
    }

.. _structure-events:

Events
======

Events are convenience interfaces with the EVM logging facilities.

::

    pragma solidity >=0.4.21 <0.7.0;

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

    pragma solidity >=0.4.0 <0.7.0;

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

    pragma solidity >=0.4.0 <0.7.0;

    contract Purchase {
        enum State { Created, Locked, Inactive } // Enum
    }
