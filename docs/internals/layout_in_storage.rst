.. index:: storage, state variable, mapping

************************************
Layout of State Variables in Storage
************************************

.. _storage-inplace-encoding:

Statically-sized variables (everything except mapping and dynamically-sized
array types) are laid out contiguously in storage starting from position ``0``.
Multiple, contiguous items that need less than 32 bytes are packed into a single
storage slot if possible, according to the following rules:

- The first item in a storage slot is stored lower-order aligned.
- Elementary types use only as many bytes as are necessary to store them.
- If an elementary type does not fit the remaining part of a storage slot, it is moved to the next storage slot.
- Structs and array data always start a new slot and occupy whole slots
  (but items inside a struct or array are packed tightly according to these rules).

For contracts that use inheritance, the ordering of state variables is determined by the
C3-linearized order of contracts starting with the most base-ward contract. If allowed
by the above rules, state variables from different contracts do share the same storage slot.

The elements of structs and arrays are stored after each other, just as if they were given explicitly.

.. warning::
    When using elements that are smaller than 32 bytes, your contract's gas usage may be higher.
    This is because the EVM operates on 32 bytes at a time. Therefore, if the element is smaller
    than that, the EVM must use more operations in order to reduce the size of the element from 32
    bytes to the desired size.

    It is only beneficial to use reduced-size arguments if you are dealing with storage values
    because the compiler will pack multiple elements into one storage slot, and thus, combine
    multiple reads or writes into a single operation. When dealing with function arguments or memory
    values, there is no inherent benefit because the compiler does not pack these values.

    Finally, in order to allow the EVM to optimize for this, ensure that you try to order your
    storage variables and ``struct`` members such that they can be packed tightly. For example,
    declaring your storage variables in the order of ``uint128, uint128, uint256`` instead of
    ``uint128, uint256, uint128``, as the former will only take up two slots of storage whereas the
    latter will take up three.

.. note::
     The layout of state variables in storage is considered to be part of the external interface
     of Solidity due to the fact that storage pointers can be passed to libraries. This means that
     any change to the rules outlined in this section is considered a breaking change
     of the language and due to its critical nature should be considered very carefully before
     being executed.


Mappings and Dynamic Arrays
===========================

.. _storage-hashed-encoding:

Due to their unpredictable size, mapping and dynamically-sized array types use a Keccak-256 hash
computation to find the starting position of the value or the array data.
These starting positions are always full stack slots.

The mapping or the dynamic array itself occupies a slot in storage at some position ``p``
according to the above rule (or by recursively applying this rule for
mappings of mappings or arrays of arrays). For dynamic arrays,
this slot stores the number of elements in the array (byte arrays and
strings are an exception, see :ref:`below <bytes-and-string>`).
For mappings, the slot is unused (but it is needed so that two equal mappings after each other will use a different
hash distribution). Array data is located at ``keccak256(p)`` and the value corresponding to a mapping key
``k`` is located at ``keccak256(k . p)`` where ``.`` is concatenation. If the value is again a
non-elementary type, the positions are found by adding an offset of ``keccak256(k . p)``.

So for the following contract snippet
the position of ``data[4][9].b`` is at ``keccak256(uint256(9) . keccak256(uint256(4) . uint256(1))) + 1``::


    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.7.0;


    contract C {
        struct S { uint a; uint b; }
        uint x;
        mapping(uint => mapping(uint => S)) data;
    }

.. _bytes-and-string:

``bytes`` and ``string``
------------------------

``bytes`` and ``string`` are encoded identically. For short byte arrays, they store their data in the same
slot where the length is also stored. In particular: if the data is at most ``31`` bytes long, it is stored
in the higher-order bytes (left aligned) and the lowest-order byte stores ``length * 2``.
For byte arrays that store data which is ``32`` or more bytes long, the main slot stores ``length * 2 + 1`` and the data is
stored as usual in ``keccak256(slot)``. This means that you can distinguish a short array from a long array
by checking if the lowest bit is set: short (not set) and long (set).

.. note::
  Handling invalidly encoded slots is currently not supported but may be added in the future.

JSON Output
===========

.. _storage-layout-top-level:

The storage layout of a contract can be requested via
the :ref:`standard JSON interface <compiler-api>`.  The output is a JSON object containing two keys,
``storage`` and ``types``.  The ``storage`` object is an array where each
element has the following form:


.. code::


    {
        "astId": 2,
        "contract": "fileA:A",
        "label": "x",
        "offset": 0,
        "slot": "0",
        "type": "t_uint256"
    }

The example above is the storage layout of ``contract A { uint x; }`` from source unit ``fileA``
and

- ``astId`` is the id of the AST node of the state variable's declaration
- ``contract`` is the name of the contract including its path as prefix
- ``label`` is the name of the state variable
- ``offset`` is the offset in bytes within the storage slot according to the encoding
- ``slot`` is the storage slot where the state variable resides or starts. This
  number may be very large and therefore its JSON value is represented as a
  string.
- ``type`` is an identifier used as key to the variable's type information (described in the following)

The given ``type``, in this case ``t_uint256`` represents an element in
``types``, which has the form:


.. code::

    {
        "encoding": "inplace",
        "label": "uint256",
        "numberOfBytes": "32",
    }

where

- ``encoding`` how the data is encoded in storage, where the possible values are:

  - ``inplace``: data is laid out contiguously in storage (see :ref:`above <storage-inplace-encoding>`).
  - ``mapping``: Keccak-256 hash-based method (see :ref:`above <storage-hashed-encoding>`).
  - ``dynamic_array``: Keccak-256 hash-based method (see :ref:`above <storage-hashed-encoding>`).
  - ``bytes``: single slot or Keccak-256 hash-based depending on the data size (see :ref:`above <bytes-and-string>`).

- ``label`` is the canonical type name.
- ``numberOfBytes`` is the number of used bytes (as a decimal string).
  Note that if ``numberOfBytes > 32`` this means that more than one slot is used.

Some types have extra information besides the four above. Mappings contain
its ``key`` and ``value`` types (again referencing an entry in this mapping
of types), arrays have its ``base`` type, and structs list their ``members`` in
the same format as the top-level ``storage`` (see :ref:`above
<storage-layout-top-level>`).

.. note ::
  The JSON output format of a contract's storage layout is still considered experimental
  and is subject to change in non-breaking releases of Solidity.

The following example shows a contract and its storage layout, containing
value and reference types, types that are encoded packed, and nested types.


.. code::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.7.0;
    contract A {
        struct S {
            uint128 a;
            uint128 b;
            uint[2] staticArray;
            uint[] dynArray;
        }

        uint x;
        uint y;
        S s;
        address addr;
        mapping (uint => mapping (address => bool)) map;
        uint[] array;
        string s1;
        bytes b1;
    }

.. code::

    "storageLayout": {
      "storage": [
        {
          "astId": 14,
          "contract": "fileA:A",
          "label": "x",
          "offset": 0,
          "slot": "0",
          "type": "t_uint256"
        },
        {
          "astId": 16,
          "contract": "fileA:A",
          "label": "y",
          "offset": 0,
          "slot": "1",
          "type": "t_uint256"
        },
        {
          "astId": 18,
          "contract": "fileA:A",
          "label": "s",
          "offset": 0,
          "slot": "2",
          "type": "t_struct(S)12_storage"
        },
        {
          "astId": 20,
          "contract": "fileA:A",
          "label": "addr",
          "offset": 0,
          "slot": "6",
          "type": "t_address"
        },
        {
          "astId": 26,
          "contract": "fileA:A",
          "label": "map",
          "offset": 0,
          "slot": "7",
          "type": "t_mapping(t_uint256,t_mapping(t_address,t_bool))"
        },
        {
          "astId": 29,
          "contract": "fileA:A",
          "label": "array",
          "offset": 0,
          "slot": "8",
          "type": "t_array(t_uint256)dyn_storage"
        },
        {
          "astId": 31,
          "contract": "fileA:A",
          "label": "s1",
          "offset": 0,
          "slot": "9",
          "type": "t_string_storage"
        },
        {
          "astId": 33,
          "contract": "fileA:A",
          "label": "b1",
          "offset": 0,
          "slot": "10",
          "type": "t_bytes_storage"
        }
      ],
      "types": {
        "t_address": {
          "encoding": "inplace",
          "label": "address",
          "numberOfBytes": "20"
        },
        "t_array(t_uint256)2_storage": {
          "base": "t_uint256",
          "encoding": "inplace",
          "label": "uint256[2]",
          "numberOfBytes": "64"
        },
        "t_array(t_uint256)dyn_storage": {
          "base": "t_uint256",
          "encoding": "dynamic_array",
          "label": "uint256[]",
          "numberOfBytes": "32"
        },
        "t_bool": {
          "encoding": "inplace",
          "label": "bool",
          "numberOfBytes": "1"
        },
        "t_bytes_storage": {
          "encoding": "bytes",
          "label": "bytes",
          "numberOfBytes": "32"
        },
        "t_mapping(t_address,t_bool)": {
          "encoding": "mapping",
          "key": "t_address",
          "label": "mapping(address => bool)",
          "numberOfBytes": "32",
          "value": "t_bool"
        },
        "t_mapping(t_uint256,t_mapping(t_address,t_bool))": {
          "encoding": "mapping",
          "key": "t_uint256",
          "label": "mapping(uint256 => mapping(address => bool))",
          "numberOfBytes": "32",
          "value": "t_mapping(t_address,t_bool)"
        },
        "t_string_storage": {
          "encoding": "bytes",
          "label": "string",
          "numberOfBytes": "32"
        },
        "t_struct(S)12_storage": {
          "encoding": "inplace",
          "label": "struct A.S",
          "members": [
            {
              "astId": 2,
              "contract": "fileA:A",
              "label": "a",
              "offset": 0,
              "slot": "0",
              "type": "t_uint128"
            },
            {
              "astId": 4,
              "contract": "fileA:A",
              "label": "b",
              "offset": 16,
              "slot": "0",
              "type": "t_uint128"
            },
            {
              "astId": 8,
              "contract": "fileA:A",
              "label": "staticArray",
              "offset": 0,
              "slot": "1",
              "type": "t_array(t_uint256)2_storage"
            },
            {
              "astId": 11,
              "contract": "fileA:A",
              "label": "dynArray",
              "offset": 0,
              "slot": "3",
              "type": "t_array(t_uint256)dyn_storage"
            }
          ],
          "numberOfBytes": "128"
        },
        "t_uint128": {
          "encoding": "inplace",
          "label": "uint128",
          "numberOfBytes": "16"
        },
        "t_uint256": {
          "encoding": "inplace",
          "label": "uint256",
          "numberOfBytes": "32"
        }
      }
    }
