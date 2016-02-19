.. index:: contract, state variable, function, event, struct, enum, function;modifier

.. _contract_structure:

***********************
Structure of a Contract
***********************

Contracts in Solidity are similar to classes in object-oriented languages.
Each contract can contain declarations of :ref:`state-variables`, :ref:`functions`,
:ref:`function-modifiers`, :ref:`events`, :ref:`structs-types` and :ref:`enum-types`.
Furthermore, contracts can inherit from other contracts.

.. _state-variables:

State Variables
===============

State variables are values which are permanently stored in contract storage.

::

  contract SimpleStorage {
    uint storedData; // State variable
    // ...
  }

.. _functions:

Functions
=========

Functions are the executable units of code within a contract.

::

  contract SimpleAuction {
    function bid() { // Function
      // ...
    }
  }

.. _function-modifiers:

Function Modifiers
==================

Function modifiers can be used to amend the semantics of functions in a declarative way.

::
  
  contract Purchase {
    address public seller;
    
    modifier onlySeller() { // Modifier
        if (msg.sender != seller) throw;
        _
    }
    
    function abort() onlySeller { // Modifier usage
        // ...
    }
  }

.. _events:

Events
======

Events are convenience interfaces with the EVM logging facilities.

::

  contract SimpleAuction {
    event HighestBidIncreased(address bidder, uint amount); // Event
    
    function bid() {
      // ...
      HighestBidIncreased(msg.sender, msg.value); // Triggering event
    }
  }

.. _structs-types:

Structs Types
=============

Structs are custom defined types that can group several variables.

::

  contract Ballot {
    struct Voter { // Struct
      uint weight;
      bool voted;
      address delegate;
      uint vote;
    }
  }

.. _enum-types:

Enum Types
==========

Enums can be used to create custom types with a finite set of values.

::
  
  contract Purchase {
    enum State { Created, Locked, Inactive } // Enum
  }
