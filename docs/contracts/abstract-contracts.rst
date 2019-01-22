.. index:: ! contract;abstract, ! abstract contract

.. _abstract-contract:

******************
Abstract Contracts
******************

Contracts are marked as abstract when at least one of their functions lacks an implementation as in the following example (note that the function declaration header is terminated by ``;``)::

    pragma solidity >=0.4.0 <0.6.0;

    contract Feline {
        function utterance() public returns (bytes32);
    }

Such contracts cannot be compiled (even if they contain implemented functions alongside non-implemented functions), but they can be used as base contracts::

    pragma solidity >=0.4.0 <0.6.0;

    contract Feline {
        function utterance() public returns (bytes32);
    }

    contract Cat is Feline {
        function utterance() public returns (bytes32) { return "miaow"; }
    }

If a contract inherits from an abstract contract and does not implement all non-implemented functions by overriding, it will itself be abstract.

Note that a function without implementation is different from a :ref:`Function Type <function_types>` even though their syntax looks very similar.

Example of function without implementation (a function declaration)::

    function foo(address) external returns (address);

Example of a Function Type (a variable declaration, where the variable is of type ``function``)::

    function(address) external returns (address) foo;

Abstract contracts decouple the definition of a contract from its implementation providing better extensibility and self-documentation and
facilitating patterns like the `Template method <https://en.wikipedia.org/wiki/Template_method_pattern>`_ and removing code duplication.
Abstract contracts are useful in the same way that defining methods in an interface is useful. It is a way for the designer of the abstract contract to say "any child of mine must implement this method".
