********************************
Solidity v0.6.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.6.0, along with the reasoning behind the changes and how to update
affected code.
For the full list check
`the release changelog <https://github.com/ethereum/solidity/releases/tag/v0.6.0>`_.


Syntactic Only Changes
======================

This section lists purely syntactic changes that do not affect the behavior of existing code.

* Conversions from external function types to ``address`` are now disallowed. Instead external
  function types have a member called ``address``, similar to the existing ``selector`` member.
* Conversions from ``address`` to ``address payable`` are now possible via ``payable(x)``, where
  ``x`` must be of type ``address``.

* Function ``push(value)`` for dynamic storage arrays do not return the new length anymore.

* New reserved keywords: ``virtual``.

Semantic Only Changes
=====================

This section lists the changes that are semantic-only, thus potentially
hiding new and different behavior in existing code.


Semantic and Syntactic Changes
==============================

This section highlights changes that affect syntax and semantics.

* The resulting type of an exponentiation is the type of the base. It used to be the smallest type
  that can hold both the type of the base and the type of the exponent, as with symmentric
  operations. Additionally, signed types are allowed for the base of the exponetation.


How to update your code
=======================

This section gives detailed instructions on how to update prior code for every breaking change.

* Change ``address(f)`` to ``f.address`` for ``f`` being of external function type.

* Change ``uint length = array.push(value)`` to ``array.push(value);``. The new length can be
  accessed via ``array.length``.

New Features
============

 * The :ref:`try/catch statement <try-catch>` allows you to react on failed external calls.
 * Yul and Inline Assembly have a new statement called ``leave`` that exits the current function.


Deprecated Elements
===================

This section lists changes that deprecate prior features or syntax.


.. _interoperability_060:

Interoperability With Older Contracts
=====================================

