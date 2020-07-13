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
* Change types of right operand in shift operators to unsigned types. For example change ``x >> (256 - y)`` to
  ``x >> uint(256 - y)``.
* Repeat the ``using A for B`` statements in all derived contracts if needed.
* Remove the ``public`` keyword from every constructor.
* Remove the ``internal`` keyword from every constructor and add ``abstract`` to the contract (if not already present).
* Change ``_slot`` and ``_offset`` suffixes in inline assembly to ``.slot`` and ``.offset``, respectively.
