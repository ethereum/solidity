.. index:: ! type;reference, ! reference type, storage, memory, location, array, struct

.. _reference-types:

Reference Types
===============

Values of reference type can be modified through multiple different names.
Contrast this with value types where you get an independent copy whenever
a variable of value type is used. Because of that, reference types have to be handled
more carefully than value types. Currently, reference types comprise structs,
arrays and mappings. If you use a reference type, you always have to explicitly
provide the data area where the type is stored: ``memory`` (whose lifetime is limited
to an external function call), ``storage`` (the location where the state variables
are stored, where the lifetime is limited to the lifetime of a contract)
or ``calldata`` (special data location that contains the function arguments).

An assignment or type conversion that changes the data location will always incur an automatic copy operation,
while assignments inside the same data location only copy in some cases for storage types.

.. _data-location:

Data location
-------------

Every reference type has an additional
annotation, the "data location", about where it is stored. There are three data locations:
``memory``, ``storage`` and ``calldata``. Calldata is a non-modifiable,
non-persistent area where function arguments are stored, and behaves mostly like memory.

.. note::
    If you can, try to use ``calldata`` as data location because it will avoid copies and
    also makes sure that the data cannot be modified. Arrays and structs with ``calldata``
    data location can also be returned from functions, but it is not possible to
    allocate such types.

.. note::
    Prior to version 0.6.9 data location for reference-type arguments was limited to
    ``calldata`` in external functions, ``memory`` in public functions and either
    ``memory`` or ``storage`` in internal and private ones.
    Now ``memory`` and ``calldata`` are allowed in all functions regardless of their visibility.

.. note::
    Prior to version 0.5.0 the data location could be omitted, and would default to different locations
    depending on the kind of variable, function type, etc., but all complex types must now give an explicit
    data location.

.. _data-location-assignment:

Data location and assignment behaviour
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Data locations are not only relevant for persistency of data, but also for the semantics of assignments:

* Assignments between ``storage`` and ``memory`` (or from ``calldata``)
  always create an independent copy.
* Assignments from ``memory`` to ``memory`` only create references. This means
  that changes to one memory variable are also visible in all other memory
  variables that refer to the same data.
* Assignments from ``storage`` to a **local** storage variable also only
  assign a reference.
* All other assignments to ``storage`` always copy. Examples for this
  case are assignments to state variables or to members of local
  variables of storage struct type, even if the local variable
  itself is just a reference.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        // The data location of x is storage.
        // This is the only place where the
        // data location can be omitted.
        uint[] x;

        // The data location of memoryArray is memory.
        function f(uint[] memory memoryArray) public {
            x = memoryArray; // works, copies the whole array to storage
            uint[] storage y = x; // works, assigns a pointer, data location of y is storage
            y[7]; // fine, returns the 8th element
            y.pop(); // fine, modifies x through y
            delete x; // fine, clears the array, also modifies y
            // The following does not work; it would need to create a new temporary /
            // unnamed array in storage, but storage is "statically" allocated:
            // y = memoryArray;
            // This does not work either, since it would "reset" the pointer, but there
            // is no sensible location it could point to.
            // delete y;
            g(x); // calls g, handing over a reference to x
            h(x); // calls h and creates an independent, temporary copy in memory
        }

        function g(uint[] storage) internal pure {}
        function h(uint[] memory) public pure {}
    }

.. index:: ! array

.. _arrays:

Arrays
------

Arrays can have a compile-time fixed size, or they can have a dynamic size.

The type of an array of fixed size ``k`` and element type ``T`` is written as ``T[k]``,
and an array of dynamic size as ``T[]``.

For example, an array of 5 dynamic arrays of ``uint`` is written as
``uint[][5]``. The notation is reversed compared to some other languages. In
Solidity, ``X[3]`` is always an array containing three elements of type ``X``,
even if ``X`` is itself an array. This is not the case in other languages such
as C.

Indices are zero-based, and access is in the opposite direction of the
declaration.

For example, if you have a variable ``uint[][5] memory x``, you access the
seventh ``uint`` in the third dynamic array using ``x[2][6]``, and to access the
third dynamic array, use ``x[2]``. Again,
if you have an array ``T[5] a`` for a type ``T`` that can also be an array,
then ``a[2]`` always has type ``T``.

Array elements can be of any type, including mapping or struct. The general
restrictions for types apply, in that mappings can only be stored in the
``storage`` data location and publicly-visible functions need parameters that are :ref:`ABI types <ABI>`.

It is possible to mark state variable arrays ``public`` and have Solidity create a :ref:`getter <visibility-and-getters>`.
The numeric index becomes a required parameter for the getter.

Accessing an array past its end causes a failing assertion. Methods ``.push()`` and ``.push(value)`` can be used
to append a new element at the end of the array, where ``.push()`` appends a zero-initialized element and returns
a reference to it.

.. index:: ! string, ! bytes

.. _strings:

.. _bytes:

``bytes`` and ``string`` as Arrays
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Variables of type ``bytes`` and ``string`` are special arrays. The ``bytes`` type is similar to ``bytes1[]``,
but it is packed tightly in calldata and memory. ``string`` is equal to ``bytes`` but does not allow
length or index access.

Solidity does not have string manipulation functions, but there are
third-party string libraries. You can also compare two strings by their keccak256-hash using
``keccak256(abi.encodePacked(s1)) == keccak256(abi.encodePacked(s2))`` and
concatenate two strings using ``string.concat(s1, s2)``.

You should use ``bytes`` over ``bytes1[]`` because it is cheaper,
since using ``bytes1[]`` in ``memory`` adds 31 padding bytes between the elements. Note that in ``storage``, the
padding is absent due to tight packing, see :ref:`bytes and string <bytes-and-string>`. As a general rule,
use ``bytes`` for arbitrary-length raw byte data and ``string`` for arbitrary-length
string (UTF-8) data. If you can limit the length to a certain number of bytes,
always use one of the value types ``bytes1`` to ``bytes32`` because they are much cheaper.

.. note::
    If you want to access the byte-representation of a string ``s``, use
    ``bytes(s).length`` / ``bytes(s)[7] = 'x';``. Keep in mind
    that you are accessing the low-level bytes of the UTF-8 representation,
    and not the individual characters.

.. index:: ! bytes-concat, ! string-concat

.. _bytes-concat:
.. _string-concat:

The functions ``bytes.concat`` and ``string.concat``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You can concatenate an arbitrary number of ``string`` values using ``string.concat``.
The function returns a single ``string memory`` array that contains the contents of the arguments without padding.
If you want to use parameters of other types that are not implicitly convertible to ``string``, you need to convert them to ``string`` first.

Analogously, the ``bytes.concat`` function can concatenate an arbitrary number of ``bytes`` or ``bytes1 ... bytes32`` values.
The function returns a single ``bytes memory`` array that contains the contents of the arguments without padding.
If you want to use string parameters or other types that are not implicitly convertible to ``bytes``, you need to convert them to ``bytes`` or ``bytes1``/.../``bytes32`` first.


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.12;

    contract C {
        string s = "Storage";
        function f(bytes calldata bc, string memory sm, bytes16 b) public view {
            string memory concatString = string.concat(s, string(bc), "Literal", sm);
            assert((bytes(s).length + bc.length + 7 + bytes(sm).length) == bytes(concatString).length);

            bytes memory concatBytes = bytes.concat(bytes(s), bc, bc[:2], "Literal", bytes(sm), b);
            assert((bytes(s).length + bc.length + 2 + 7 + bytes(sm).length + b.length) == concatBytes.length);
        }
    }

If you call ``string.concat`` or ``bytes.concat`` without arguments they return an empty array.

.. index:: ! array;allocating, new

Allocating Memory Arrays
^^^^^^^^^^^^^^^^^^^^^^^^

Memory arrays with dynamic length can be created using the ``new`` operator.
As opposed to storage arrays, it is **not** possible to resize memory arrays (e.g.
the ``.push`` member functions are not available).
You either have to calculate the required size in advance
or create a new memory array and copy every element.

As all variables in Solidity, the elements of newly allocated arrays are always initialized
with the :ref:`default value<default-value>`.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f(uint len) public pure {
            uint[] memory a = new uint[](7);
            bytes memory b = new bytes(len);
            assert(a.length == 7);
            assert(b.length == len);
            a[6] = 8;
        }
    }

.. index:: ! array;literals, ! inline;arrays

Array Literals
^^^^^^^^^^^^^^

An array literal is a comma-separated list of one or more expressions, enclosed
in square brackets (``[...]``). For example ``[1, a, f(3)]``. The type of the
array literal is determined as follows:

It is always a statically-sized memory array whose length is the
number of expressions.

The base type of the array is the type of the first expression on the list such that all
other expressions can be implicitly converted to it. It is a type error
if this is not possible.

It is not enough that there is a type all the elements can be converted to. One of the elements
has to be of that type.

In the example below, the type of ``[1, 2, 3]`` is
``uint8[3] memory``, because the type of each of these constants is ``uint8``. If
you want the result to be a ``uint[3] memory`` type, you need to convert
the first element to ``uint``.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f() public pure {
            g([uint(1), 2, 3]);
        }
        function g(uint[3] memory) public pure {
            // ...
        }
    }

The array literal ``[1, -1]`` is invalid because the type of the first expression
is ``uint8`` while the type of the second is ``int8`` and they cannot be implicitly
converted to each other. To make it work, you can use ``[int8(1), -1]``, for example.

Since fixed-size memory arrays of different type cannot be converted into each other
(even if the base types can), you always have to specify a common base type explicitly
if you want to use two-dimensional array literals:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f() public pure returns (uint24[2][4] memory) {
            uint24[2][4] memory x = [[uint24(0x1), 1], [0xffffff, 2], [uint24(0xff), 3], [uint24(0xffff), 4]];
            // The following does not work, because some of the inner arrays are not of the right type.
            // uint[2][4] memory x = [[0x1, 1], [0xffffff, 2], [0xff, 3], [0xffff, 4]];
            return x;
        }
    }

Fixed size memory arrays cannot be assigned to dynamically-sized
memory arrays, i.e. the following is not possible:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    // This will not compile.
    contract C {
        function f() public {
            // The next line creates a type error because uint[3] memory
            // cannot be converted to uint[] memory.
            uint[] memory x = [uint(1), 3, 4];
        }
    }

It is planned to remove this restriction in the future, but it creates some
complications because of how arrays are passed in the ABI.

If you want to initialize dynamically-sized arrays, you have to assign the
individual elements:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f() public pure {
            uint[] memory x = new uint[](3);
            x[0] = 1;
            x[1] = 3;
            x[2] = 4;
        }
    }

.. index:: ! array;length, length, push, pop, !array;push, !array;pop

.. _array-members:

Array Members
^^^^^^^^^^^^^

**length**:
    Arrays have a ``length`` member that contains their number of elements.
    The length of memory arrays is fixed (but dynamic, i.e. it can depend on
    runtime parameters) once they are created.
**push()**:
     Dynamic storage arrays and ``bytes`` (not ``string``) have a member function
     called ``push()`` that you can use to append a zero-initialised element at the end of the array.
     It returns a reference to the element, so that it can be used like
     ``x.push().t = 2`` or ``x.push() = b``.
**push(x)**:
     Dynamic storage arrays and ``bytes`` (not ``string``) have a member function
     called ``push(x)`` that you can use to append a given element at the end of the array.
     The function returns nothing.
**pop()**:
     Dynamic storage arrays and ``bytes`` (not ``string``) have a member
     function called ``pop()`` that you can use to remove an element from the
     end of the array. This also implicitly calls :ref:`delete<delete>` on the removed element. The function returns nothing.

.. note::
    Increasing the length of a storage array by calling ``push()``
    has constant gas costs because storage is zero-initialised,
    while decreasing the length by calling ``pop()`` has a
    cost that depends on the "size" of the element being removed.
    If that element is an array, it can be very costly, because
    it includes explicitly clearing the removed
    elements similar to calling :ref:`delete<delete>` on them.

.. note::
    To use arrays of arrays in external (instead of public) functions, you need to
    activate ABI coder v2.

.. note::
    In EVM versions before Byzantium, it was not possible to access
    dynamic arrays return from function calls. If you call functions
    that return dynamic arrays, make sure to use an EVM that is set to
    Byzantium mode.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract ArrayContract {
        uint[2**20] aLotOfIntegers;
        // Note that the following is not a pair of dynamic arrays but a
        // dynamic array of pairs (i.e. of fixed size arrays of length two).
        // Because of that, T[] is always a dynamic array of T, even if T
        // itself is an array.
        // Data location for all state variables is storage.
        bool[2][] pairsOfFlags;

        // newPairs is stored in memory - the only possibility
        // for public contract function arguments
        function setAllFlagPairs(bool[2][] memory newPairs) public {
            // assignment to a storage array performs a copy of ``newPairs`` and
            // replaces the complete array ``pairsOfFlags``.
            pairsOfFlags = newPairs;
        }

        struct StructType {
            uint[] contents;
            uint moreInfo;
        }
        StructType s;

        function f(uint[] memory c) public {
            // stores a reference to ``s`` in ``g``
            StructType storage g = s;
            // also changes ``s.moreInfo``.
            g.moreInfo = 2;
            // assigns a copy because ``g.contents``
            // is not a local variable, but a member of
            // a local variable.
            g.contents = c;
        }

        function setFlagPair(uint index, bool flagA, bool flagB) public {
            // access to a non-existing index will throw an exception
            pairsOfFlags[index][0] = flagA;
            pairsOfFlags[index][1] = flagB;
        }

        function changeFlagArraySize(uint newSize) public {
            // using push and pop is the only way to change the
            // length of an array
            if (newSize < pairsOfFlags.length) {
                while (pairsOfFlags.length > newSize)
                    pairsOfFlags.pop();
            } else if (newSize > pairsOfFlags.length) {
                while (pairsOfFlags.length < newSize)
                    pairsOfFlags.push();
            }
        }

        function clear() public {
            // these clear the arrays completely
            delete pairsOfFlags;
            delete aLotOfIntegers;
            // identical effect here
            pairsOfFlags = new bool[2][](0);
        }

        bytes byteData;

        function byteArrays(bytes memory data) public {
            // byte arrays ("bytes") are different as they are stored without padding,
            // but can be treated identical to "uint8[]"
            byteData = data;
            for (uint i = 0; i < 7; i++)
                byteData.push();
            byteData[3] = 0x08;
            delete byteData[2];
        }

        function addFlag(bool[2] memory flag) public returns (uint) {
            pairsOfFlags.push(flag);
            return pairsOfFlags.length;
        }

        function createMemoryArray(uint size) public pure returns (bytes memory) {
            // Dynamic memory arrays are created using `new`:
            uint[2][] memory arrayOfPairs = new uint[2][](size);

            // Inline arrays are always statically-sized and if you only
            // use literals, you have to provide at least one type.
            arrayOfPairs[0] = [uint(1), 2];

            // Create a dynamic byte array:
            bytes memory b = new bytes(200);
            for (uint i = 0; i < b.length; i++)
                b[i] = bytes1(uint8(i));
            return b;
        }
    }

.. index:: ! array;dangling storage references

Dangling References to Storage Array Elements
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When working with storage arrays, you need to take care to avoid dangling references.
A dangling reference is a reference that points to something that no longer exists or has been
moved without updating the reference. A dangling reference can for example occur, if you store a
reference to an array element in a local variable and then ``.pop()`` from the containing array:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0 <0.9.0;

    contract C {
        uint[][] s;

        function f() public {
            // Stores a pointer to the last array element of s.
            uint[] storage ptr = s[s.length - 1];
            // Removes the last array element of s.
            s.pop();
            // Writes to the array element that is no longer within the array.
            ptr.push(0x42);
            // Adding a new element to ``s`` now will not add an empty array, but
            // will result in an array of length 1 with ``0x42`` as element.
            s.push();
            assert(s[s.length - 1][0] == 0x42);
        }
    }

The write in ``ptr.push(0x42)`` will **not** revert, despite the fact that ``ptr`` no
longer refers to a valid element of ``s``. Since the compiler assumes that unused storage
is always zeroed, a subsequent ``s.push()`` will not explicitly write zeroes to storage,
so the last element of ``s`` after that ``push()`` will have length ``1`` and contain
``0x42`` as its first element.

Note that Solidity does not allow to declare references to value types in storage. These kinds
of explicit dangling references are restricted to nested reference types. However, dangling references
can also occur temporarily when using complex expressions in tuple assignments:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0 <0.9.0;

    contract C {
        uint[] s;
        uint[] t;
        constructor() {
            // Push some initial values to the storage arrays.
            s.push(0x07);
            t.push(0x03);
        }

        function g() internal returns (uint[] storage) {
            s.pop();
            return t;
        }

        function f() public returns (uint[] memory) {
            // The following will first evaluate ``s.push()`` to a reference to a new element
            // at index 1. Afterwards, the call to ``g`` pops this new element, resulting in
            // the left-most tuple element to become a dangling reference. The assignment still
            // takes place and will write outside the data area of ``s``.
            (s.push(), g()[0]) = (0x42, 0x17);
            // A subsequent push to ``s`` will reveal the value written by the previous
            // statement, i.e. the last element of ``s`` at the end of this function will have
            // the value ``0x42``.
            s.push();
            return s;
        }
    }

It is always safer to only assign to storage once per statement and to avoid
complex expressions on the left-hand-side of an assignment.

You need to take particular care when dealing with references to elements of
``bytes`` arrays, since a ``.push()`` on a bytes array may switch :ref:`from short
to long layout in storage<bytes-and-string>`.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0 <0.9.0;

    // This will report a warning
    contract C {
        bytes x = "012345678901234567890123456789";

        function test() external returns(uint) {
            (x.push(), x.push()) = (0x01, 0x02);
            return x.length;
        }
    }

Here, when the first ``x.push()`` is evaluated, ``x`` is still stored in short
layout, thereby ``x.push()`` returns a reference to an element in the first storage slot of
``x``. However, the second ``x.push()`` switches the bytes array to large layout.
Now the element that ``x.push()`` referred to is in the data area of the array while
the reference still points at its original location, which is now a part of the length field
and the assignment will effectively garble the length of ``x``.
To be safe, only enlarge bytes arrays by at most one element during a single
assignment and do not simultaneously index-access the array in the same statement.

While the above describes the behaviour of dangling storage references in the
current version of the compiler, any code with dangling references should be
considered to have *undefined behaviour*. In particular, this means that
any future version of the compiler may change the behaviour of code that
involves dangling references.

Be sure to avoid dangling references in your code!

.. index:: ! array;slice

.. _array-slices:

Array Slices
------------


Array slices are a view on a contiguous portion of an array.
They are written as ``x[start:end]``, where ``start`` and
``end`` are expressions resulting in a uint256 type (or
implicitly convertible to it). The first element of the
slice is ``x[start]`` and the last element is ``x[end - 1]``.

If ``start`` is greater than ``end`` or if ``end`` is greater
than the length of the array, an exception is thrown.

Both ``start`` and ``end`` are optional: ``start`` defaults
to ``0`` and ``end`` defaults to the length of the array.

Array slices do not have any members. They are implicitly
convertible to arrays of their underlying type
and support index access. Index access is not absolute
in the underlying array, but relative to the start of
the slice.

Array slices do not have a type name which means
no variable can have an array slices as type,
they only exist in intermediate expressions.

.. note::
    As of now, array slices are only implemented for calldata arrays.

Array slices are useful to ABI-decode secondary data passed in function parameters:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.5 <0.9.0;
    contract Proxy {
        /// @dev Address of the client contract managed by proxy i.e., this contract
        address client;

        constructor(address client_) {
            client = client_;
        }

        /// Forward call to "setOwner(address)" that is implemented by client
        /// after doing basic validation on the address argument.
        function forward(bytes calldata payload) external {
            bytes4 sig = bytes4(payload[:4]);
            // Due to truncating behaviour, bytes4(payload) performs identically.
            // bytes4 sig = bytes4(payload);
            if (sig == bytes4(keccak256("setOwner(address)"))) {
                address owner = abi.decode(payload[4:], (address));
                require(owner != address(0), "Address of owner cannot be zero.");
            }
            (bool status,) = client.delegatecall(payload);
            require(status, "Forwarded call failed.");
        }
    }



.. index:: ! struct, ! type;struct

.. _structs:

Structs
-------

Solidity provides a way to define new types in the form of structs, which is
shown in the following example:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    // Defines a new type with two fields.
    // Declaring a struct outside of a contract allows
    // it to be shared by multiple contracts.
    // Here, this is not really needed.
    struct Funder {
        address addr;
        uint amount;
    }

    contract CrowdFunding {
        // Structs can also be defined inside contracts, which makes them
        // visible only there and in derived contracts.
        struct Campaign {
            address payable beneficiary;
            uint fundingGoal;
            uint numFunders;
            uint amount;
            mapping (uint => Funder) funders;
        }

        uint numCampaigns;
        mapping (uint => Campaign) campaigns;

        function newCampaign(address payable beneficiary, uint goal) public returns (uint campaignID) {
            campaignID = numCampaigns++; // campaignID is return variable
            // We cannot use "campaigns[campaignID] = Campaign(beneficiary, goal, 0, 0)"
            // because the right hand side creates a memory-struct "Campaign" that contains a mapping.
            Campaign storage c = campaigns[campaignID];
            c.beneficiary = beneficiary;
            c.fundingGoal = goal;
        }

        function contribute(uint campaignID) public payable {
            Campaign storage c = campaigns[campaignID];
            // Creates a new temporary memory struct, initialised with the given values
            // and copies it over to storage.
            // Note that you can also use Funder(msg.sender, msg.value) to initialise.
            c.funders[c.numFunders++] = Funder({addr: msg.sender, amount: msg.value});
            c.amount += msg.value;
        }

        function checkGoalReached(uint campaignID) public returns (bool reached) {
            Campaign storage c = campaigns[campaignID];
            if (c.amount < c.fundingGoal)
                return false;
            uint amount = c.amount;
            c.amount = 0;
            c.beneficiary.transfer(amount);
            return true;
        }
    }

The contract does not provide the full functionality of a crowdfunding
contract, but it contains the basic concepts necessary to understand structs.
Struct types can be used inside mappings and arrays and they can themselves
contain mappings and arrays.

It is not possible for a struct to contain a member of its own type,
although the struct itself can be the value type of a mapping member
or it can contain a dynamically-sized array of its type.
This restriction is necessary, as the size of the struct has to be finite.

Note how in all the functions, a struct type is assigned to a local variable
with data location ``storage``.
This does not copy the struct but only stores a reference so that assignments to
members of the local variable actually write to the state.

Of course, you can also directly access the members of the struct without
assigning it to a local variable, as in
``campaigns[campaignID].amount = 0``.

.. note::
    Until Solidity 0.7.0, memory-structs containing members of storage-only types (e.g. mappings)
    were allowed and assignments like ``campaigns[campaignID] = Campaign(beneficiary, goal, 0, 0)``
    in the example above would work and just silently skip those members.
