.. index:: ! contract

.. _contracts:

##########
Contracts
##########

Contracts in Solidity are similar to classes in object-oriented languages. They
contain persistent data in state variables and functions that can modify these
variables. Calling a function on a different contract (instance) will perform
an EVM function call and thus switch the context such that state variables are
inaccessible.

.. include:: conracts/creating-contracts.rst
.. include:: conracts/visibility-and-getters.rst
.. include:: conracts/function-modifiers.rst
.. include:: conracts/constat-state-variables.rst
.. include:: conracts/functions.rst
.. include:: conracts/events.rst
.. include:: conracts/inheritance.rst
.. include:: conracts/abstract-contracts.rst
.. include:: conracts/interfaces.rst
.. include:: conracts/libraries.rst
.. include:: conracts/using-for.rst