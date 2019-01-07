.. index:: ! constant

************************
Constant State Variables
************************

State variables can be declared as ``constant``. In this case, they have to be
assigned from an expression which is a constant at compile time. Any expression
that accesses storage, blockchain data (e.g. ``now``, ``address(this).balance`` or
``block.number``) or
execution data (``msg.value`` or ``gasleft()``) or makes calls to external contracts is disallowed. Expressions
that might have a side-effect on memory allocation are allowed, but those that
might have a side-effect on other memory objects are not. The built-in functions
``keccak256``, ``sha256``, ``ripemd160``, ``ecrecover``, ``addmod`` and ``mulmod``
are allowed (even though, with the exception of ``keccak256``, they do call external contracts).

The reason behind allowing side-effects on the memory allocator is that it
should be possible to construct complex objects like e.g. lookup-tables.
This feature is not yet fully usable.

The compiler does not reserve a storage slot for these variables, and every occurrence is
replaced by the respective constant expression (which might be computed to a single value by the optimizer).

Not all types for constants are implemented at this time. The only supported types are
value types and strings.

::

    pragma solidity >=0.4.0 <0.6.0;

    contract C {
        uint constant x = 32**22 + 8;
        string constant text = "abc";
        bytes32 constant myHash = keccak256("abc");
    }
