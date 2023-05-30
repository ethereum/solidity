.. index:: ! type;conversion, ! cast

.. _types-conversion-elementary-types:

Conversions between Elementary Types
====================================

Implicit Conversions
--------------------

An implicit type conversion is automatically applied by the compiler in some cases
during assignments, when passing arguments to functions and when applying operators.
In general, an implicit conversion between value-types is possible if it makes
sense semantically and no information is lost.

For example, ``uint8`` is convertible to
``uint16`` and ``int128`` to ``int256``, but ``int8`` is not convertible to ``uint256``,
because ``uint256`` cannot hold values such as ``-1``.

If an operator is applied to different types, the compiler tries to implicitly
convert one of the operands to the type of the other (the same is true for assignments).
This means that operations are always performed in the type of one of the operands.

For more details about which implicit conversions are possible,
please consult the sections about the types themselves.

In the example below, ``y`` and ``z``, the operands of the addition,
do not have the same type, but ``uint8`` can
be implicitly converted to ``uint16`` and not vice-versa. Because of that,
``y`` is converted to the type of ``z`` before the addition is performed
in the ``uint16`` type. The resulting type of the expression ``y + z`` is ``uint16``.
Because it is assigned to a variable of type ``uint32`` another implicit conversion
is performed after the addition.

.. code-block:: solidity

    uint8 y;
    uint16 z;
    uint32 x = y + z;


Explicit Conversions
--------------------

If the compiler does not allow implicit conversion but you are confident a conversion will work,
an explicit type conversion is sometimes possible. This may
result in unexpected behavior and allows you to bypass some security
features of the compiler, so be sure to test that the
result is what you want and expect!

Take the following example that converts a negative ``int`` to a ``uint``:

.. code-block:: solidity

    int  y = -3;
    uint x = uint(y);

At the end of this code snippet, ``x`` will have the value ``0xfffff..fd`` (64 hex
characters), which is -3 in the two's complement representation of 256 bits.

If an integer is explicitly converted to a smaller type, higher-order bits are
cut off:

.. code-block:: solidity

    uint32 a = 0x12345678;
    uint16 b = uint16(a); // b will be 0x5678 now

If an integer is explicitly converted to a larger type, it is padded on the left (i.e., at the higher order end).
The result of the conversion will compare equal to the original integer:

.. code-block:: solidity

    uint16 a = 0x1234;
    uint32 b = uint32(a); // b will be 0x00001234 now
    assert(a == b);

Fixed-size bytes types behave differently during conversions. They can be thought of as
sequences of individual bytes and converting to a smaller type will cut off the
sequence:

.. code-block:: solidity

    bytes2 a = 0x1234;
    bytes1 b = bytes1(a); // b will be 0x12

If a fixed-size bytes type is explicitly converted to a larger type, it is padded on
the right. Accessing the byte at a fixed index will result in the same value before and
after the conversion (if the index is still in range):

.. code-block:: solidity

    bytes2 a = 0x1234;
    bytes4 b = bytes4(a); // b will be 0x12340000
    assert(a[0] == b[0]);
    assert(a[1] == b[1]);

Since integers and fixed-size byte arrays behave differently when truncating or
padding, explicit conversions between integers and fixed-size byte arrays are only allowed,
if both have the same size. If you want to convert between integers and fixed-size byte arrays of
different size, you have to use intermediate conversions that make the desired truncation and padding
rules explicit:

.. code-block:: solidity

    bytes2 a = 0x1234;
    uint32 b = uint16(a); // b will be 0x00001234
    uint32 c = uint32(bytes4(a)); // c will be 0x12340000
    uint8 d = uint8(uint16(a)); // d will be 0x34
    uint8 e = uint8(bytes1(a)); // e will be 0x12

``bytes`` arrays and ``bytes`` calldata slices can be converted explicitly to fixed bytes types (``bytes1``/.../``bytes32``).
In case the array is longer than the target fixed bytes type, truncation at the end will happen.
If the array is shorter than the target type, it will be padded with zeros at the end.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.5;

    contract C {
        bytes s = "abcdefgh";
        function f(bytes calldata c, bytes memory m) public view returns (bytes16, bytes3) {
            require(c.length == 16, "");
            bytes16 b = bytes16(m);  // if length of m is greater than 16, truncation will happen
            b = bytes16(s);  // padded on the right, so result is "abcdefgh\0\0\0\0\0\0\0\0"
            bytes3 b1 = bytes3(s); // truncated, b1 equals to "abc"
            b = bytes16(c[:8]);  // also padded with zeros
            return (b, b1);
        }
    }

.. index:: ! literal;conversion, literal;rational, literal;hexadecimal number
.. _types-conversion-literals:

Conversions between Literals and Elementary Types
=================================================

Integer Types
-------------

Decimal and hexadecimal number literals can be implicitly converted to any integer type
that is large enough to represent it without truncation:

.. code-block:: solidity

    uint8 a = 12; // fine
    uint32 b = 1234; // fine
    uint16 c = 0x123456; // fails, since it would have to truncate to 0x3456

.. note::
    Prior to version 0.8.0, any decimal or hexadecimal number literals could be explicitly
    converted to an integer type. From 0.8.0, such explicit conversions are as strict as implicit
    conversions, i.e., they are only allowed if the literal fits in the resulting range.

.. index:: literal;string, literal;hexadecimal

Fixed-Size Byte Arrays
----------------------

Decimal number literals cannot be implicitly converted to fixed-size byte arrays. Hexadecimal
number literals can be, but only if the number of hex digits exactly fits the size of the bytes
type. As an exception both decimal and hexadecimal literals which have a value of zero can be
converted to any fixed-size bytes type:

.. code-block:: solidity

    bytes2 a = 54321; // not allowed
    bytes2 b = 0x12; // not allowed
    bytes2 c = 0x123; // not allowed
    bytes2 d = 0x1234; // fine
    bytes2 e = 0x0012; // fine
    bytes4 f = 0; // fine
    bytes4 g = 0x0; // fine

String literals and hex string literals can be implicitly converted to fixed-size byte arrays,
if their number of characters matches the size of the bytes type:

.. code-block:: solidity

    bytes2 a = hex"1234"; // fine
    bytes2 b = "xy"; // fine
    bytes2 c = hex"12"; // not allowed
    bytes2 d = hex"123"; // not allowed
    bytes2 e = "x"; // not allowed
    bytes2 f = "xyz"; // not allowed

.. index:: literal;address

Addresses
---------

As described in :ref:`address_literals`, hex literals of the correct size that pass the checksum
test are of ``address`` type. No other literals can be implicitly converted to the ``address`` type.

Explicit conversions to ``address`` are allowed only from ``bytes20`` and ``uint160``.

An ``address a`` can be converted explicitly to ``address payable`` via ``payable(a)``.

.. note::
    Prior to version 0.8.0, it was possible to explicitly convert from any integer type (of any size, signed or unsigned) to  ``address`` or ``address payable``.
    Starting with in 0.8.0 only conversion from ``uint160`` is allowed.