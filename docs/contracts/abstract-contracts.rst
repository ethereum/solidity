.. index:: ! contract;abstract, ! abstract contract

.. _abstract-contract:

******************
Abstract Contracts
******************

Contracts need to be marked as abstract when at least one of their functions is not implemented.
Contracts may be marked as abstract even though all functions are implemented.

This can be done by using the ``abstract`` keyword as shown in the following example. Note that this contract needs to be
defined as abstract, because the function ``utterance()`` was defined, but no implementation was
provided (no implementation body ``{ }`` was given).::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.8.0;

    abstract contract Feline {
        function utterance() public virtual returns (bytes32);
    }

Such abstract contracts can not be instantiated directly. This is also true, if an abstract contract itself does implement
all defined functions. The usage of an abstract contract as a base class is shown in the following example::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.8.0;

    abstract contract Feline {
        function utterance() public pure virtual returns (bytes32);
    }

    contract Cat is Feline {
        function utterance() public pure override returns (bytes32) { return "miaow"; }
    }

If a contract inherits from an abstract contract and does not implement all non-implemented
functions by overriding, it needs to be marked as abstract as well.

Note that a function without implementation is different from
a :ref:`Function Type <function_types>` even though their syntax looks very similar.

Example of function without implementation (a function declaration)::

    function foo(address) external returns (address);

Example of a declaration of a variable whose type is a function type::

    function(address) external returns (address) foo;

Abstract contracts decouple the definition of a contract from its
implementation providing better extensibility and self-documentation and
facilitating patterns like the `Template method <https://en.wikipedia.org/wiki/Template_method_pattern>`_ and removing code duplication.
Abstract contracts are useful in the same way that defining methods
in an interface is useful. It is a way for the designer of the
abstract contract to say "any child of mine must implement this method".

.. note::

  Abstract contracts cannot override an implemented virtual function with an
  unimplemented one.
