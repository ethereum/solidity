.. index:: type

.. _types:

*****
Types
*****

Solidity is a statically typed language, which means that the type of each
variable (state and local) needs to be specified.
Solidity provides several elementary types which can be combined to form complex types.

In addition, types can interact with each other in expressions containing
operators. For a quick reference of the various operators, see :ref:`order`.

The concept of "undefined" or "null" values does not exist in Solidity, but newly
declared variables always have a :ref:`default value<default-value>` dependent
on its type. To handle any unexpected values, you should use the :ref:`revert function<assert-and-require>` to revert the whole transaction, or return a
tuple with a second `bool` value denoting success.

.. include:: types/value-types.rst

.. include:: types/reference-types.rst

.. index:: !mapping
.. _mapping-types:

Mapping Types
=============

You declare mapping types with the syntax ``mapping(_KeyType => _ValueType)``.
The ``_KeyType`` can be any elementary type. This means it can be any of
the built-in value types plus ``bytes`` and ``string``. User-defined
or complex types like contract types, enums, mappings, structs and any array type
apart from ``bytes`` and ``string`` are not allowed.
``_ValueType`` can be any type, including mappings.

You can think of mappings as `hash tables <https://en.wikipedia.org/wiki/Hash_table>`_, which are virtually initialised
such that every possible key exists and is mapped to a value whose
byte-representation is all zeros, a type's :ref:`default value <default-value>`. The similarity ends there, the key data is not stored in a
mapping, only its ``keccak256`` hash is used to look up the value.

Because of this, mappings do not have a length or a concept of a key or
value being set.

Mappings can only have a data location of ``storage`` and thus
are allowed for state variables, as storage reference types
in functions, or as parameters for library functions.
They cannot be used as parameters or return parameters
of contract functions that are publicly visible.

You can mark variables of mapping type as ``public`` and Solidity creates a
:ref:`getter <visibility-and-getters>` for you. The ``_KeyType`` becomes a
parameter for the getter. If ``_ValueType`` is a value type or a struct,
the getter returns ``_ValueType``.
If ``_ValueType`` is an array or a mapping, the getter has one parameter for
each ``_KeyType``, recursively. For example with a mapping:

::

    pragma solidity >=0.4.0 <0.6.0;

    contract MappingExample {
        mapping(address => uint) public balances;

        function update(uint newBalance) public {
            balances[msg.sender] = newBalance;
        }
    }

    contract MappingUser {
        function f() public returns (uint) {
            MappingExample m = new MappingExample();
            m.update(100);
            return m.balances(address(this));
        }
    }


.. note::
  Mappings are not iterable, but it is possible to implement a data structure
  on top of them. For an example, see `iterable mapping <https://github.com/ethereum/dapp-bin/blob/master/library/iterable_mapping.sol>`_.

.. index:: assignment, ! delete, lvalue

Operators Involving LValues
===========================

If ``a`` is an LValue (i.e. a variable or something that can be assigned to), the following operators are available as shorthands:

``a += e`` is equivalent to ``a = a + e``. The operators ``-=``, ``*=``, ``/=``, ``%=``, ``|=``, ``&=`` and ``^=`` are defined accordingly. ``a++`` and ``a--`` are equivalent to ``a += 1`` / ``a -= 1`` but the expression itself still has the previous value of ``a``. In contrast, ``--a`` and ``++a`` have the same effect on ``a`` but return the value after the change.

delete
------

``delete a`` assigns the initial value for the type to ``a``. I.e. for integers it is
equivalent to ``a = 0``, but it can also be used on arrays, where it assigns a dynamic
array of length zero or a static array of the same length with all elements set to their
initial value. ``delete a[x]`` deletes the item at index ``x`` of the array and leaves
all other elements and the length of the array untouched. This especially means that it leaves
a gap in the array. If you plan to remove items, a mapping is probably a better choice.

For structs, it assigns a struct with all members reset. In other words, the value of ``a`` after ``delete a`` is the same as if ``a`` would be declared without assignment, with the following caveat:

``delete`` has no effect on mappings (as the keys of mappings may be arbitrary and are generally unknown). So if you delete a struct, it will reset all members that are not mappings and also recurse into the members unless they are mappings. However, individual keys and what they map to can be deleted: If ``a`` is a mapping, then ``delete a[x]`` will delete the value stored at ``x``.

It is important to note that ``delete a`` really behaves like an assignment to ``a``, i.e. it stores a new object in ``a``.
This distinction is visible when ``a`` is reference variable: It will only reset ``a`` itself, not the
value it referred to previously.

::

    pragma solidity >=0.4.0 <0.6.0;

    contract DeleteExample {
        uint data;
        uint[] dataArray;

        function f() public {
            uint x = data;
            delete x; // sets x to 0, does not affect data
            delete data; // sets data to 0, does not affect x
            uint[] storage y = dataArray;
            delete dataArray; // this sets dataArray.length to zero, but as uint[] is a complex object, also
            // y is affected which is an alias to the storage object
            // On the other hand: "delete y" is not valid, as assignments to local variables
            // referencing storage objects can only be made from existing storage objects.
            assert(y.length == 0);
        }
    }

.. index:: ! type;conversion, ! cast

.. _types-conversion-elementary-types:

Conversions between Elementary Types
====================================

Implicit Conversions
--------------------

If an operator is applied to different types, the compiler tries to
implicitly convert one of the operands to the type of the other (the same is
true for assignments). In general, an implicit conversion between value-types
is possible if it
makes sense semantically and no information is lost: ``uint8`` is convertible to
``uint16`` and ``int128`` to ``int256``, but ``int8`` is not convertible to ``uint256``
(because ``uint256`` cannot hold e.g. ``-1``).

For more details, please consult the sections about the types themselves.

Explicit Conversions
--------------------

If the compiler does not allow implicit conversion but you know what you are
doing, an explicit type conversion is sometimes possible. Note that this may
give you some unexpected behaviour and allows you to bypass some security
features of the compiler, so be sure to test that the
result is what you want! Take the following example where you are converting
a negative ``int8`` to a ``uint``:

::

    int8 y = -3;
    uint x = uint(y);

At the end of this code snippet, ``x`` will have the value ``0xfffff..fd`` (64 hex
characters), which is -3 in the two's complement representation of 256 bits.

If an integer is explicitly converted to a smaller type, higher-order bits are
cut off::

    uint32 a = 0x12345678;
    uint16 b = uint16(a); // b will be 0x5678 now

If an integer is explicitly converted to a larger type, it is padded on the left (i.e. at the higher order end).
The result of the conversion will compare equal to the original integer::

    uint16 a = 0x1234;
    uint32 b = uint32(a); // b will be 0x00001234 now
    assert(a == b);

Fixed-size bytes types behave differently during conversions. They can be thought of as
sequences of individual bytes and converting to a smaller type will cut off the
sequence::

    bytes2 a = 0x1234;
    bytes1 b = bytes1(a); // b will be 0x12

If a fixed-size bytes type is explicitly converted to a larger type, it is padded on
the right. Accessing the byte at a fixed index will result in the same value before and
after the conversion (if the index is still in range)::

    bytes2 a = 0x1234;
    bytes4 b = bytes4(a); // b will be 0x12340000
    assert(a[0] == b[0]);
    assert(a[1] == b[1]);

Since integers and fixed-size byte arrays behave differently when truncating or
padding, explicit conversions between integers and fixed-size byte arrays are only allowed,
if both have the same size. If you want to convert between integers and fixed-size byte arrays of
different size, you have to use intermediate conversions that make the desired truncation and padding
rules explicit::

    bytes2 a = 0x1234;
    uint32 b = uint16(a); // b will be 0x00001234
    uint32 c = uint32(bytes4(a)); // c will be 0x12340000
    uint8 d = uint8(uint16(a)); // d will be 0x34
    uint8 e = uint8(bytes1(a)); // e will be 0x12

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
