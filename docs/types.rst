.. index:: type

*****
Types
*****

Solidity is a statically typed language, which means that the type of each
variable (state and local) needs to be specified (or at least known -
see :ref:`type-deduction` below) at
compile-time. Solidity provides several elementary types which can be combined
to complex types.

.. index:: ! value type, ! type;value

Value Types
===========

The following types are also called value types because variables of these
types will always be passed by value, i.e. they are always copied when they
are used as function arguments or in assignments.

.. index:: ! bool, ! true, ! false

Booleans
--------

`bool`: The possible values are constants `true` and `false`.

Operators:  

*  `!` (logical negation)
*  `&&` (logical conjunction, "and")
*  `||` (logical disjunction, "or")
*  `==` (equality)
*  `!=` (inequality)

The operators `||` and `&&` apply the common short-circuiting rules. This means that in the expression `f(x) || g(y)`, if `f(x)` evaluates to `true`, `g(y)` will not be evaluated even if it may have side-effects.

.. index:: ! uint, ! int, ! integer

Integers
--------

`int•` / `uint•`: Signed and unsigned integers of various sizes. Keywords `uint8` to `uint256` in steps of `8` (unsigned of 8 up to 256 bits) and `int8` to `int256`. `uint` and `int` are aliases for `uint256` and `int256`, respectively.

Operators:  

* Comparisons: `<=`, `<`, `==`, `!=`, `>=`, `>` (evaluate to `bool`)  
* Bit operators: `&`, `|`, `^` (bitwise exclusive or), `~` (bitwise negation)  
* Arithmetic operators: `+`, `-`, unary `-`, unary `+`, `*`, `/`, `%` (remainder), `**` (exponentiation)

.. index:: address, balance, send, call, callcode

Address
-------

`address`: Holds a 20 byte value (size of an Ethereum address). Address types also have members(see [Functions on addresses](#functions-on-addresses)) and serve as base for all contracts.

Operators:  

* `<=`, `<`, `==`, `!=`, `>=` and `>`

Members of Addresses
^^^^^^^^^^^^^^^^^^^^

* `balance` and `send`

It is possible to query the balance of an address using the property `balance`
and to send Ether (in units of wei) to an address using the `send` function:

::

    address x = 0x123;
    address myAddress = this;
    if (x.balance < 10 && myAddress.balance >= 10) x.send(10);

.. note::
    If `x` is a contract address, its code (more specifically: its fallback function, if present) will be executed together with the `send` call (this is a limitation of the EVM and cannot be prevented). If that execution runs out of gas or fails in any way, the Ether transfer will be reverted. In this case, `send` returns `false`.

* `call` and `callcode`

Furthermore, to interface with contracts that do not adhere to the ABI,
the function `call` is provided which takes an arbitrary number of arguments of any type. These arguments are padded to 32 bytes and concatenated. One exception is the case where the first argument is encoded to exactly four bytes. In this case, it is not padded to allow the use of function signatures here.

::

    address nameReg = 0x72ba7d8e73fe8eb666ea66babc8116a41bfb10e2;
    nameReg.call("register", "MyName");
    nameReg.call(bytes4(sha3("fun(uint256)")), a);

`call` returns a boolean indicating whether the invoked function terminated (`true`) or caused an EVM exception (`false`). It is not possible to access the actual data returned (for this we would need to know the encoding and size in advance).

In a similar way, the function `callcode` can be used: The difference is that only the code of the given address is used, all other aspects (storage, balance, ...) are taken from the current contract. The purpose of `callcode` is to use library code which is stored in another contract. The user has to ensure that the layout of storage in both contracts is suitable for callcode to be used.

Both `call` and `callcode` are very low-level functions and should only be used as a *last resort* as they break the type-safety of Solidity.

.. note::
    All contracts inherit the members of address, so it is possible to query the balance of the
    current contract using `this.balance`.

.. index:: byte array, bytes32


Fixed-size byte arrays
----------------------

`bytes1`, `bytes2`, `bytes3`, ..., `bytes32`. `byte` is an alias for `bytes1`.  

Operators:  

* Comparisons: `<=`, `<`, `==`, `!=`, `>=`, `>` (evaluate to `bool`)  
* Bit operators: `&`, `|`, `^` (bitwise exclusive or), `~` (bitwise negation)  

Dynamically-sized byte array
----------------------------

`bytes`:
    Dynamically-sized byte array, see :ref:`arrays`. Not a value-type!  
`string`:
    Dynamically-sized UTF8-encoded string, see :ref:`arrays`. Not a value-type!

As a rule of thumb, use `bytes` for arbitrary-length raw byte data and `string`
for arbitrary-length string (utf-8) data. If you can limit the length to a certain
number of bytes, always use one of `bytes1` to `bytes32` because they are much cheaper.

.. index:: literal, literal;integer

Integer Literals
-----------------

Integer Literals are arbitrary precision integers until they are used together with a non-literal. In `var x = 1 - 2;`, for example, the value of `1 - 2` is `-1`, which is assigned to `x` and thus `x` receives the type `int8` -- the smallest type that contains `-1`, although the natural types of `1` and `2` are actually `uint8`.    

It is even possible to temporarily exceed the maximum of 256 bits as long as only integer literals are used for the computation: `var x = (0xffffffffffffffffffff * 0xffffffffffffffffffff) * 0;` Here, `x` will have the value `0` and thus the type `uint8`.

.. index:: literal, literal;string, string

String Literals
---------------

String Literals are written with double quotes (`"abc"`). As with integer literals, their type can vary, but they are implicitly convertible to `bytes•` if they fit, to `bytes` and to `string`.

.. index:: enum

Enums
=====

Enums are one way to create a user-defined type in Solidity. They are explicitly convertible
to and from all integer types but implicit conversion is not allowed.

::

    contract test {
        enum ActionChoices { GoLeft, GoRight, GoStraight, SitStill }
        ActionChoices choice;
        ActionChoices constant defaultChoice = ActionChoices.GoStraight;
        function setGoStraight()
        {
            choice = ActionChoices.GoStraight;
        }
        // Since enum types are not part of the ABI, the signature of "getChoice"
        // will automatically be changed to "getChoice() returns (uint8)"
        // for all matters external to Solidity. The integer type used is just
        // large enough to hold all enum values, i.e. if you have more values,
        // `uint16` will be used and so on.
        function getChoice() returns (ActionChoices)
        {
            return choice;
        }
        function getDefaultChoice() returns (uint)
        {
            return uint(defaultChoice);
        }
    }

.. index:: ! type;reference, ! reference type, storage, memory, location, array, struct

Reference Types
==================

Complex types, i.e. types which do not always fit into 256 bits have to be handled
more carefully than the value-types we have already seen. Since copying
them can be quite expensive, we have to think about whether we want them to be
stored in **memory** (which is not persisting) or **storage** (where the state
variables are held).

Data location
-------------

Every complex type, i.e. *arrays* and *structs*, has an additional
annotation, the "data location", about whether it is stored in memory or in storage. Depending on the
context, there is always a default, but it can be overridden by appending
either `storage` or `memory` to the type. The default for function parameters (including return parameters) is `memory`, the default for local variables is `storage` and the location is forced
to `storage` for state variables (obviously).

There is also a third data location, "calldata", which is a non-modifyable
non-persistent area where function arguments are stored. Function parameters
(not return parameters) of external functions are forced to "calldata" and
it behaves mostly like memory.

Data locations are important because they change how assignments behave:
Assignments between storage and memory and also to a state variable (even from other state variables)
always create an independent copy.
Assignments to local storage variables only assign a reference though, and
this reference always points to the state variable even if the latter is changed
in the meantime.
On the other hand, assignments from a memory stored reference type to another
memory-stored reference type does not create a copy.

::

    contract c {
      uint[] x; // the data location of x is storage
      // the data location of memoryArray is memory
      function f(uint[] memoryArray) {
        x = memoryArray; // works, copies the whole array to storage
        var y = x; // works, assigns a pointer, data location of y is storage
        y[7]; // fine, returns the 8th element
        y.length = 2; // fine, modifies x through y
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
      function g(uint[] storage storageArray) internal {}
      function h(uint[] memoryArray) {}
    }

Summary
^^^^^^^

Forced data location:
 - parameters (not return) of external functions: calldata
 - state variables: storage

Default data location:
 - parameters (also return) of functions: memory
 - all other local variables: storage

.. index:: ! array

.. _arrays:

Arrays
------

Arrays can have a compile-time fixed size or they can be dynamic.
For storage arrays, the element type can be arbitrary (i.e. also other
arrays, mappings or structs). For memory arrays, it cannot be a mapping and
has to be an ABI type if it is an argument of a publicly-visible function.

An array of fixed size `k` and element type `T` is written as `T[k]`,
an array of dynamic size as `T[]`. As an example, an array of 5 dynamic
arrays of `uint` is `uint[][5]` (note that the notation is reversed when
compared to some other languages). To access the second uint in the
third dynamic array, you use `x[2][1]` (indices are zero-based and
access works in the opposite way of the declaration, i.e. `x[2]`
shaves off one level in the type from the right).

Variables of type `bytes` and `string` are special arrays. A `bytes` is similar to `byte[]`,
but it is packed tightly in calldata. `string` is equal to `bytes` but does not allow
length or index access (for now).

So `bytes` should always be preferred over `byte[]` because it is cheaper.

.. note::
    If you want to access the byte-representation of a string `s`, use
    `bytes(s).length` / `bytes(s)[7] = 'x';`. Keep in mind
    that you are accessing the low-level bytes of the utf-8 representation,
    and not the individual characters!

.. index:: ! array;length, length, push, !array;push

Members
^^^^^^^

**length**:
    Arrays have a `length` member to hold their number of elements.
    Dynamic arrays can be resized in storage (not in memory) by changing the
    `.length` member. This does not happen automatically when attempting to access elements outside the current length. The size of memory arrays is fixed (but dynamic, i.e. it can depend on runtime parameters) once they are created.
**push**:
     Dynamic storage arrays and `bytes` (not `string`) have a member function called `push` that can be used to append an element at the end of the array. The function returns the new length.

.. warning::
    It is not yet possible to use arrays of arrays in external functions.

.. warning::
    Due to limitations of the EVM, it is not possible to return
    dynamic content from external function calls. The function `f` in
    `contract C { function f() returns (uint[]) { ... } }` will return
    something if called from web3.js, but not if called from Solidity.

    The only workaround for now is to use large statically-sized arrays.


::

    contract ArrayContract {
      uint[2**20] m_aLotOfIntegers;
      // Note that the following is not a pair of arrays but an array of pairs.
      bool[2][] m_pairsOfFlags;
      // newPairs is stored in memory - the default for function arguments
      function setAllFlagPairs(bool[2][] newPairs) {
        // assignment to a storage array replaces the complete array
        m_pairsOfFlags = newPairs;
      }
      function setFlagPair(uint index, bool flagA, bool flagB) {
        // access to a non-existing index will throw an exception
        m_pairsOfFlags[index][0] = flagA;
        m_pairsOfFlags[index][1] = flagB;
      }
      function changeFlagArraySize(uint newSize) {
        // if the new size is smaller, removed array elements will be cleared
        m_pairsOfFlags.length = newSize;
      }
      function clear() {
        // these clear the arrays completely
        delete m_pairsOfFlags;
        delete m_aLotOfIntegers;
        // identical effect here
        m_pairsOfFlags.length = 0;
      }
      bytes m_byteData;
      function byteArrays(bytes data) {
        // byte arrays ("bytes") are different as they are stored without padding,
        // but can be treated identical to "uint8[]"
        m_byteData = data;
        m_byteData.length += 7;
        m_byteData[3] = 8;
        delete m_byteData[2];
      }
      function addFlag(bool[2] flag) returns (uint) {
        return m_pairsOfFlags.push(flag);
      }
      function createMemoryArray(uint size) returns (bytes) {
        // Dynamic memory arrays are created using `new`:
        uint[2][] memory arrayOfPairs = new uint[2][](size);
        // Create a dynamic byte array:
        bytes memory b = new bytes(200);
        for (uint i = 0; i < b.length; i++)
          b[i] = byte(i);
        return b;
      }
    }


.. index:: ! struct, ! type;struct

Structs
-------

Solidity provides a way to define new types in the form of structs, which is
shown in the following example:

::

    contract CrowdFunding {
      // Defines a new type with two fields.
      struct Funder {
        address addr;
        uint amount;
      }
      struct Campaign {
        address beneficiary;
        uint fundingGoal;
        uint numFunders;
        uint amount;
        mapping (uint => Funder) funders;
      }
      uint numCampaigns;
      mapping (uint => Campaign) campaigns;
      function newCampaign(address beneficiary, uint goal) returns (uint campaignID) {
        campaignID = numCampaigns++; // campaignID is return variable
        // Creates new struct and saves in storage. We leave out the mapping type.
        campaigns[campaignID] = Campaign(beneficiary, goal, 0, 0);
      }
      function contribute(uint campaignID) {
        Campaign c = campaigns[campaignID];
            // Creates a new temporary memory struct, initialised with the given values
            // and copies it over to storage.
            // Note that you can also use Funder(msg.sender, msg.value) to initialise.
        c.funders[c.numFunders++] = Funder({addr: msg.sender, amount: msg.value});
        c.amount += msg.value;
      }
      function checkGoalReached(uint campaignID) returns (bool reached) {
        Campaign c = campaigns[campaignID];
        if (c.amount < c.fundingGoal)
          return false;
        c.beneficiary.send(c.amount);
        c.amount = 0;
        return true;
      }
    }

The contract does not provide the full functionality of a crowdfunding
contract, but it contains the basic concepts necessary to understand structs.
Struct types can be used inside mappings and arrays and they can itself
contain mappings and arrays.

It is not possible for a struct to contain a member of its own type,
although the struct itself can be the value type of a mapping member.
This restriction is necessary, as the size of the struct has to be finite.

Note how in all the functions, a struct type is assigned to a local variable
(of the default storage data location).
This does not copy the struct but only stores a reference so that assignments to
members of the local variable actually write to the state.

Of course, you can also directly access the members of the struct without
assigning it to a local variable, as in
`campaigns[campaignID].amount = 0`.

.. index:: !mapping

Mappings
========

Mapping types are declared as `mapping _KeyType => _ValueType`, where
`_KeyType` can be almost any type except for a mapping and `_ValueType`
can actually be any type, including mappings.

Mappings can be seen as hashtables which are virtually initialized such that
every possible key exists and is mapped to a value whose byte-representation is
all zeros. The similarity ends here, though: The key data is not actually stored
in a mapping, only its `sha3` hash used to look up the value.

Because of this, mappings do not have a length or a concept of a key or value being "set".

Mappings are only allowed for state variables (or as storage reference types
in internal functions).

.. index:: assignment, ! delete, lvalue

Operators Involving LValues
===========================

If `a` is an LValue (i.e. a variable or something that can be assigned to), the following operators are available as shorthands:

`a += e` is equivalent to `a = a + e`. The operators `-=`, `*=`, `/=`, `%=`, `a |=`, `&=` and `^=` are defined accordingly. `a++` and `a--` are equivalent to `a += 1` / `a -= 1` but the expression itself still has the previous value of `a`. In contrast, `--a` and `++a` have the same effect on `a` but return the value after the change.

delete
------

`delete a` assigns the initial value for the type to `a`. I.e. for integers it is equivalent to `a = 0`, but it can also be used on arrays, where it assigns a dynamic array of length zero or a static array of the same length with all elements reset. For structs, it assigns a struct with all members reset.

`delete` has no effect on whole mappings (as the keys of mappings may be arbitrary and are generally unknown). So if you delete a struct, it will reset all members that are not mappings and also recurse into the members unless they are mappings. However, individual keys and what they map to can be deleted.

It is important to note that `delete a` really behaves like an assignment to `a`, i.e. it stores a new object in `a`.

::

    contract DeleteExample {
      uint data;
      uint[] dataArray;
      function f() {
        uint x = data;
        delete x; // sets x to 0, does not affect data
        delete data; // sets data to 0, does not affect x which still holds a copy
        uint[] y = dataArray;
        delete dataArray; // this sets dataArray.length to zero, but as uint[] is a complex object, also
        // y is affected which is an alias to the storage object
        // On the other hand: "delete y" is not valid, as assignments to local variables
        // referencing storage objects can only be made from existing storage objects.
      }
    }

.. index:: ! type;conversion, ! cast

Conversions between Elementary Types
====================================

Implicit Conversions
--------------------

If an operator is applied to different types, the compiler tries to
implicitly convert one of the operands to the type of the other (the same is
true for assignments). In general, an implicit conversion between value-types
is possible if it
makes sense semantically and no information is lost: `uint8` is convertible to
`uint16` and `int128` to `int256`, but `int8` is not convertible to `uint256`
(because `uint256` cannot hold e.g. `-1`).
Furthermore, unsigned integers can be converted to bytes of the same or larger
size, but not vice-versa. Any type that can be converted to `uint160` can also
be converted to `address`.

Explicit Conversions
--------------------

If the compiler does not allow implicit conversion but you know what you are
doing, an explicit type conversion is sometimes possible::

    int8 y = -3;
    uint x = uint(y);

At the end of this code snippet, `x` will have the value `0xfffff..fd` (64 hex
characters), which is -3 in two's complement representation of 256 bits.

If a type is explicitly converted to a smaller type, higher-order bits are
cut off::

    uint32 a = 0x12345678;
    uint16 b = uint16(a); // b will be 0x5678 now

.. index:: ! type;deduction, ! var

.. _type-deduction:

Type Deduction
==============

For convenience, it is not always necessary to explicitly specify the type of a
variable, the compiler automatically infers it from the type of the first
expression that is assigned to the variable::

    uint20 x = 0x123;
    var y = x;

Here, the type of `y` will be `uint20`. Using `var` is not possible for function
parameters or return parameters.

.. warning::
    The type is only deduced from the first assignment, so
    the loop in the following snippet is infinite, as `i` will have the type
    `uint8` and any value of this type is smaller than `2000`.
    `for (var i = 0; i < 2000; i++) { ... }`

