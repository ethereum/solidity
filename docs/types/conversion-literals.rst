.. _types-conversion-literals:

Conversions between Literals and Elementary Types
=================================================

Integer Types
-------------

Decimal and hexadecimal number literals can be implicitly converted to any integer type
that is large enough to represent it without truncation::

    uint8 a = 12; // fine
    uint32 b = 1234; // fine
    uint16 c = 0x123456; // fails, since it would have to truncate to 0x3456

Fixed-Size Byte Arrays
----------------------

Decimal number literals cannot be implicitly converted to fixed-size byte arrays. Hexadecimal
number literals can be, but only if the number of hex digits exactly fits the size of the bytes
type. As an exception both decimal and hexadecimal literals which have a value of zero can be
converted to any fixed-size bytes type::

    bytes2 a = 54321; // not allowed
    bytes2 b = 0x12; // not allowed
    bytes2 c = 0x123; // not allowed
    bytes2 d = 0x1234; // fine
    bytes2 e = 0x0012; // fine
    bytes4 f = 0; // fine
    bytes4 g = 0x0; // fine

String literals and hex string literals can be implicitly converted to fixed-size byte arrays,
if their number of characters matches the size of the bytes type::

    bytes2 a = hex"1234"; // fine
    bytes2 b = "xy"; // fine
    bytes2 c = hex"12"; // not allowed
    bytes2 d = hex"123"; // not allowed
    bytes2 e = "x"; // not allowed
    bytes2 f = "xyz"; // not allowed

Addresses
---------

As described in :ref:`address_literals`, hex literals of the correct size that pass the checksum
test are of ``address`` type. No other literals can be implicitly converted to the ``address`` type.

Explicit conversions from ``bytes20`` or any integer type to ``address`` result in ``address payable``.