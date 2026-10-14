.. _frequently-reported-non-bugs:

############################
Frequently Reported Non-Bugs
############################

The behaviors listed below are reported as compiler bugs on a regular basis,
but are either intentional, documented, or explicitly not guaranteed by the language.

The list of actual bugs is tracked in the :ref:`list of known bugs<known_bugs>`.

Non-canonical ABI-encoded calldata is accepted
==============================================

Offsets that point backwards, overlap, or leave gaps are not necessarily rejected,
and trailing data past the encoded arguments is ignored.
The Solidity decoder does not enforce :ref:`strict encoding mode<abi_packed_mode>`;
the invariant we uphold is that decoding must not read beyond ``calldatasize()``.

Different results between the evmasm and the IR pipeline
========================================================

The two pipelines have some semantic differences.
Known and intentional divergences are listed in :ref:`Solidity IR-based Codegen Changes<ir-breaking-changes>`,
in particular :ref:`cleanup behavior<ir-cleanup>` differs.

Order of evaluation
===================

The :ref:`evaluation order of expressions is unspecified<order-of-evaluation>` and this includes the components
of tuple assignments and returns (see also :ref:`Tuples are not proper types<destructuring-assignments>`).
Only statement order and short-circuiting of boolean operators are guaranteed.
Note that :ref:`assignments involving reference types<data-location-assignment>` copy or alias depending on
data location, which is documented behavior and not an aliasing bug by itself.

Stale references into storage
=============================

A reference to a storage array element can be left dangling by anything that resizes or relocates the
containing array, for example a ``.pop()``, a ``.push()`` that switches a ``bytes`` array
:ref:`from short to long layout<bytes-and-string>`, or an assignment to the array itself as in ``(s[0], s) = (x, y)``.
Writing through such a reference is not rejected and may write outside the data area of the array.

This is :ref:`documented behavior<dangling-storage-references>`, and code containing dangling
references is explicitly considered to have *undefined behavior*.
