********************************
Solidity v0.7.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.7.0, along with the reasoning behind the changes and how to update
affected code.
For the full list check
`the release changelog <https://github.com/ethereum/solidity/releases/tag/v0.7.0>`_.

How to update your code
=======================

This section gives detailed instructions on how to update prior code for every breaking change.

* Change ``f.value(...)()`` to ``f{value: ...}()``. Similarly ``(new C).value(...)()`` to
  ``(new C){value: ...}()`` and ``f.gas(...)()`` to ``f{gas: ...}()``.
* Change ``now`` to ``block.timestamp``.
