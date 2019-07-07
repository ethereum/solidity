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

Deprecated Elements
===================

This section lists changes that deprecate prior features or syntax.


.. _interoperability_060:

Interoperability With Older Contracts
=====================================

