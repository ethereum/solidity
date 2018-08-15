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

.. index:: ! value type, ! type;value

Value Types
===========

The following types are also called value types because variables of these
types will always be passed by value, i.e. they are always copied when they
are used as function arguments or in assignments.

.. index:: ! bool, ! true, ! false

Booleans
--------

``bool``: The possible values are constants ``true`` and ``false``.

Operators:

*  ``!`` (logical negation)
*  ``&&`` (logical conjunction, "and")
*  ``||`` (logical disjunction, "or")
*  ``==`` (equality)
*  ``!=`` (inequality)

The operators ``||`` and ``&&`` apply the common short-circuiting rules. This means that in the expression ``f(x) || g(y)``, if ``f(x)`` evaluates to ``true``, ``g(y)`` will not be evaluated even if it may have side-effects.

.. index:: ! uint, ! int, ! integer

Integers
--------

``int`` / ``uint``: Signed and unsigned integers of various sizes. Keywords ``uint8`` to ``uint256`` in steps of ``8`` (unsigned of 8 up to 256 bits) and ``int8`` to ``int256``. ``uint`` and ``int`` are aliases for ``uint256`` and ``int256``, respectively.

Operators:

* Comparisons: ``<=``, ``<``, ``==``, ``!=``, ``>=``, ``>`` (evaluate to ``bool``)
* Bit operators: ``&``, ``|``, ``^`` (bitwise exclusive or), ``~`` (bitwise negation)
* Arithmetic operators: ``+``, ``-``, unary ``-``, unary ``+``, ``*``, ``/``, ``%`` (remainder), ``**`` (exponentiation), ``<<`` (left shift), ``>>`` (right shift)

Division always truncates (it is just compiled to the ``DIV`` opcode of the EVM), but it does not truncate if both
operators are :ref:`literals<rational_literals>` (or literal expressions).

Division by zero and modulus with zero throws a runtime exception.

The result of a shift operation is the type of the left operand. The
expression ``x << y`` is equivalent to ``x * 2**y``, and, for positive integers,
``x >> y`` is equivalent to ``x / 2**y``. For negative ``x``, ``x >> y``
is equivalent to dividing by a power of ``2`` while rounding down (towards negative infinity).
Shifting by a negative amount throws a runtime exception.

.. warning::
    Before version ``0.5.0`` a right shift ``x >> y`` for negative ``x`` was equivalent to ``x / 2**y``,
    i.e. right shifts used rounding towards zero instead of rounding towards negative infinity.

.. index:: ! ufixed, ! fixed, ! fixed point number

Fixed Point Numbers
-------------------

.. warning::
    Fixed point numbers are not fully supported by Solidity yet. They can be declared, but
    cannot be assigned to or from.

``fixed`` / ``ufixed``: Signed and unsigned fixed point number of various sizes. Keywords ``ufixedMxN`` and ``fixedMxN``, where ``M`` represents the number of bits taken by
the type and ``N`` represents how many decimal points are available. ``M`` must be divisible by 8 and goes from 8 to 256 bits. ``N`` must be between 0 and 80, inclusive.
``ufixed`` and ``fixed`` are aliases for ``ufixed128x18`` and ``fixed128x18``, respectively.

Operators:

* Comparisons: ``<=``, ``<``, ``==``, ``!=``, ``>=``, ``>`` (evaluate to ``bool``)
* Arithmetic operators: ``+``, ``-``, unary ``-``, unary ``+``, ``*``, ``/``, ``%`` (remainder)

.. note::
    The main difference between floating point (``float`` and ``double`` in many languages, more precisely IEEE 754 numbers) and fixed point numbers is
    that the number of bits used for the integer and the fractional part (the part after the decimal dot) is flexible in the former, while it is strictly
    defined in the latter. Generally, in floating point almost the entire space is used to represent the number, while only a small number of bits define
    where the decimal point is.

.. index:: address, balance, send, call, callcode, delegatecall, staticcall, transfer

.. _address:

Address
-------

``address``: Holds a 20 byte value (size of an Ethereum address). Address types also have members and serve as a base for all contracts.
``address payable``: Same as ``address``, but with the additional members ``transfer`` and ``send``.

Implicit conversions from ``address payable`` to ``address`` are allowed, whereas conversions from ``address`` to ``address payable`` are
not possible (the only way to perform such a conversion is by using an intermediate conversion to ``uint160``).
Conversions of the form ``address payable(x)`` are not allowed. Instead the result of a conversion of the form ``address(x)``
has the type ``address payable``, if ``x`` is of integer or fixed bytes type, a literal or a contract with a payable fallback function.
If ``x`` is a contract without payable fallback function ``address(x)`` will be of type ``address``. The type of address literals
is ``address payable``.
In external function signatures ``address`` is used for both the ``address`` and the ``address payable`` type.

Operators:

* ``<=``, ``<``, ``==``, ``!=``, ``>=`` and ``>``

.. warning::
    If you convert a type that uses a larger byte size to an ``address``, for example ``bytes32``, then the ``address`` is truncated.
    To reduce conversion ambiguity version 0.4.24 and higher of the compiler force you make the truncation explicit in the conversion.
    Take for example the address ``0x111122223333444455556666777788889999AAAABBBBCCCCDDDDEEEEFFFFCCCC``.

    You can use ``address(uint160(bytes20(b)))``, which results in ``0x111122223333444455556666777788889999aAaa``,
    or you can use ``address(uint160(uint256(b)))``, which results in ``0x777788889999AaAAbBbbCcccddDdeeeEfFFfCcCc``.

.. note::
    Starting with version 0.5.0 contracts do not derive from the address type, but can still be explicitly converted to
    ``address`` or to ``address payable``, if they have a payable fallback function.

.. _members-of-addresses:

Members of Addresses
^^^^^^^^^^^^^^^^^^^^

* ``balance`` and ``transfer``

For a quick reference, see :ref:`address_related`.

It is possible to query the balance of an address using the property ``balance``
and to send Ether (in units of wei) to a payable address using the ``transfer`` function:

::

    address payable x = address(0x123);
    address myAddress = this;
    if (x.balance < 10 && myAddress.balance >= 10) x.transfer(10);

.. note::
    If ``x`` is a contract address, its code (more specifically: its :ref:`fallback-function`, if present) will be executed together with the ``transfer`` call (this is a feature of the EVM and cannot be prevented). If that execution runs out of gas or fails in any way, the Ether transfer will be reverted and the current contract will stop with an exception.

* ``send``

Send is the low-level counterpart of ``transfer``. If the execution fails, the current contract will not stop with an exception, but ``send`` will return ``false``.

.. warning::
    There are some dangers in using ``send``: The transfer fails if the call stack depth is at 1024
    (this can always be forced by the caller) and it also fails if the recipient runs out of gas. So in order
    to make safe Ether transfers, always check the return value of ``send``, use ``transfer`` or even better:
    use a pattern where the recipient withdraws the money.

* ``call``, ``callcode``, ``delegatecall`` and ``staticcall``

Furthermore, to interface with contracts that do not adhere to the ABI,
or to get more direct control over the encoding,
the function ``call`` is provided which takes a single byte array as input.
The functions ``abi.encode``, ``abi.encodePacked``, ``abi.encodeWithSelector``
and ``abi.encodeWithSignature`` can be used to encode structured data.

.. warning::
    All these functions are low-level functions and should be used with care.
    Specifically, any unknown contract might be malicious and if you call it, you
    hand over control to that contract which could in turn call back into
    your contract, so be prepared for changes to your state variables
    when the call returns. The regular way to interact with other contracts
    is to call a function on a contract object (``x.f()``).

:: note::
    Previous versions of Solidity allowed these functions to receive
    arbitrary arguments and would also handle a first argument of type
    ``bytes4`` differently. These edge cases were removed in version 0.5.0.

``call`` returns a boolean indicating whether the invoked function terminated (``true``) or caused an EVM exception (``false``). It is not possible to access the actual data returned with plain Solidity. However, using inline assembly it is possible to make a raw ``call`` and access the actual data returned with the ``returndatacopy`` instruction.

It is possible to adjust the supplied gas with the ``.gas()`` modifier::

    namReg.call.gas(1000000)(abi.encodeWithSignature("register(string)", "MyName"));

Similarly, the supplied Ether value can be controlled too::

    nameReg.call.value(1 ether)(abi.encodeWithSignature("register(string)", "MyName"));

Lastly, these modifiers can be combined. Their order does not matter::

    nameReg.call.gas(1000000).value(1 ether)(abi.encodeWithSignature("register(string)", "MyName"));

.. note::
    It is not yet possible to use the gas or value modifiers on overloaded functions.

    A workaround is to introduce a special case for gas and value and just re-check
    whether they are present at the point of overload resolution.

In a similar way, the function ``delegatecall`` can be used: the difference is that only the code of the given address is used, all other aspects (storage, balance, ...) are taken from the current contract. The purpose of ``delegatecall`` is to use library code which is stored in another contract. The user has to ensure that the layout of storage in both contracts is suitable for delegatecall to be used. Prior to homestead, only a limited variant called ``callcode`` was available that did not provide access to the original ``msg.sender`` and ``msg.value`` values.

Since byzantium ``staticcall`` can be used as well. This is basically the same as ``call``, but will revert, if the called function modifies the state in any way.

All four functions ``call``, ``delegatecall``, ``callcode`` and ``staticcall`` are very low-level functions and should only be used as a *last resort* as they break the type-safety of Solidity.

The ``.gas()`` option is available on all three methods, while the ``.value()`` option is not supported for ``delegatecall``.

.. note::
    All contracts can be converted to ``address`` type, so it is possible to query the balance of the
    current contract using ``address(this).balance``.

.. note::
    The use of ``callcode`` is discouraged and will be removed in the future.

.. index:: ! contract type, ! type; contract

.. _contract_types:

Contract Types
--------------

Every :ref:`contract<contracts>` defines its own type.
You can implicitly convert contracts to contracts they inherit from,
and explicitly convert them to and from the ``address`` type, if they have no
payable fallback functions, or to and from the ``address payable`` type, if they do
have payable fallback functions.

.. note::
    Starting with version 0.5.0 contracts do not derive from the address type,
    but can still be explicitly converted to ``address``, resp. to ``address payable``,
    if they have a payable fallback function.

If you declare a local variable of contract type (`MyContract c`), you can call
functions on that contract. Take care to assign it from somewhere that is the
same contract type.

You can also instantiate contracts (which means they are newly created). You
can find more details in the :ref:`'Contracts via new'<creating-contracts>`
section.

The data representation of a contract is identical to that of the ``address``
type and this type is also used in the :ref:`ABI<ABI>`.

Contracts do not support any operators.

The members of contract types are the external functions of the contract
including public state variables.

.. index:: byte array, bytes32

Fixed-size byte arrays
----------------------

``bytes1``, ``bytes2``, ``bytes3``, ..., ``bytes32``. ``byte`` is an alias for ``bytes1``.

Operators:

* Comparisons: ``<=``, ``<``, ``==``, ``!=``, ``>=``, ``>`` (evaluate to ``bool``)
* Bit operators: ``&``, ``|``, ``^`` (bitwise exclusive or), ``~`` (bitwise negation), ``<<`` (left shift), ``>>`` (right shift)
* Index access: If ``x`` is of type ``bytesI``, then ``x[k]`` for ``0 <= k < I`` returns the ``k`` th byte (read-only).

The shifting operator works with any integer type as right operand (but will
return the type of the left operand), which denotes the number of bits to shift by.
Shifting by a negative amount will cause a runtime exception.

Members:

* ``.length`` yields the fixed length of the byte array (read-only).

.. note::
    It is possible to use an array of bytes as ``byte[]``, but it is wasting a lot of space, 31 bytes every element,
    to be exact, when passing in calls. It is better to use ``bytes``.

Dynamically-sized byte array
----------------------------

``bytes``:
    Dynamically-sized byte array, see :ref:`arrays`. Not a value-type!
``string``:
    Dynamically-sized UTF-8-encoded string, see :ref:`arrays`. Not a value-type!

.. index:: address, literal;address

.. _address_literals:

Address Literals
----------------

Hexadecimal literals that pass the address checksum test, for example
``0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF`` are of ``address`` type.
Hexadecimal literals that are between 39 and 41 digits
long and do not pass the checksum test produce
a warning and are treated as regular rational number literals.

.. note::
    The mixed-case address checksum format is defined in `EIP-55 <https://github.com/ethereum/EIPs/blob/master/EIPS/eip-55.md>`_.

.. index:: literal, literal;rational

.. _rational_literals:

Rational and Integer Literals
-----------------------------

Integer literals are formed from a sequence of numbers in the range 0-9.
They are interpreted as decimals. For example, ``69`` means sixty nine.
Octal literals do not exist in Solidity and leading zeros are invalid.

Decimal fraction literals are formed by a ``.`` with at least one number on
one side.  Examples include ``1.``, ``.1`` and ``1.3``.

Scientific notation is also supported, where the base can have fractions, while the exponent cannot.
Examples include ``2e10``, ``-2e10``, ``2e-10``, ``2.5e1``.

Underscores can be used to separate the digits of a numeric literal to aid readability.
For example, decimal ``123_000``, hexadecimal ``0x2eff_abde``, scientific decimal notation ``1_2e345_678`` are all valid.
Underscores are only allowed between two digits and only one consecutive underscore is allowed.
There is no additional semantic meaning added to a number literal containing underscores.

Number literal expressions retain arbitrary precision until they are converted to a non-literal type (i.e. by
using them together with a non-literal expression).
This means that computations do not overflow and divisions do not truncate
in number literal expressions.

For example, ``(2**800 + 1) - 2**800`` results in the constant ``1`` (of type ``uint8``)
although intermediate results would not even fit the machine word size. Furthermore, ``.5 * 8`` results
in the integer ``4`` (although non-integers were used in between).

Any operator that can be applied to integers can also be applied to number literal expressions as
long as the operands are integers. If any of the two is fractional, bit operations are disallowed
and exponentiation is disallowed if the exponent is fractional (because that might result in
a non-rational number).

.. note::
    Solidity has a number literal type for each rational number.
    Integer literals and rational number literals belong to number literal types.
    Moreover, all number literal expressions (i.e. the expressions that
    contain only number literals and operators) belong to number literal
    types.  So the number literal expressions ``1 + 2`` and ``2 + 1`` both
    belong to the same number literal type for the rational number three.

.. warning::
    Division on integer literals used to truncate in earlier versions, but it will now convert into a rational number, i.e. ``5 / 2`` is not equal to ``2``, but to ``2.5``.

.. note::
    Number literal expressions are converted into a non-literal type as soon as they are used with non-literal
    expressions. Even though we know that the value of the
    expression assigned to ``b`` in the following example evaluates to
    an integer, but the partial expression ``2.5 + a`` does not type check so the code
    does not compile

::

    uint128 a = 1;
    uint128 b = 2.5 + a + 0.5;

.. index:: literal, literal;string, string

String Literals
---------------

String literals are written with either double or single-quotes (``"foo"`` or ``'bar'``).  They do not imply trailing zeroes as in C; ``"foo"`` represents three bytes not four.  As with integer literals, their type can vary, but they are implicitly convertible to ``bytes1``, ..., ``bytes32``, if they fit, to ``bytes`` and to ``string``.

String literals support escape characters, such as ``\n``, ``\xNN`` and ``\uNNNN``. ``\xNN`` takes a hex value and inserts the appropriate byte, while ``\uNNNN`` takes a Unicode codepoint and inserts an UTF-8 sequence.

.. index:: literal, bytes

Hexadecimal Literals
--------------------

Hexademical Literals are prefixed with the keyword ``hex`` and are enclosed in double or single-quotes (``hex"001122FF"``). Their content must be a hexadecimal string and their value will be the binary representation of those values.

Hexademical Literals behave like String Literals and have the same convertibility restrictions.

.. index:: enum

.. _enums:

Enums
-----

Enums are one way to create a user-defined type in Solidity. They are explicitly convertible
to and from all integer types but implicit conversion is not allowed.  The explicit conversions
check the value ranges at runtime and a failure causes an exception.  Enums needs at least one member.

The data representation is the same as for enums in C: The options are represented by
subsequent unsigned integer values starting from ``0``.


::

    pragma solidity ^0.4.16;

    contract test {
        enum ActionChoices { GoLeft, GoRight, GoStraight, SitStill }
        ActionChoices choice;
        ActionChoices constant defaultChoice = ActionChoices.GoStraight;

        function setGoStraight() public {
            choice = ActionChoices.GoStraight;
        }

        // Since enum types are not part of the ABI, the signature of "getChoice"
        // will automatically be changed to "getChoice() returns (uint8)"
        // for all matters external to Solidity. The integer type used is just
        // large enough to hold all enum values, i.e. if you have more than 256 values,
        // `uint16` will be used and so on.
        function getChoice() public view returns (ActionChoices) {
            return choice;
        }

        function getDefaultChoice() public pure returns (uint) {
            return uint(defaultChoice);
        }
    }

.. index:: ! function type, ! type; function

.. _function_types:

Function Types
--------------

Function types are the types of functions. Variables of function type
can be assigned from functions and function parameters of function type
can be used to pass functions to and return functions from function calls.
Function types come in two flavours - *internal* and *external* functions:

Internal functions can only be called inside the current contract (more specifically,
inside the current code unit, which also includes internal library functions
and inherited functions) because they cannot be executed outside of the
context of the current contract. Calling an internal function is realized
by jumping to its entry label, just like when calling a function of the current
contract internally.

External functions consist of an address and a function signature and they can
be passed via and returned from external function calls.

Function types are notated as follows::

    function (<parameter types>) {internal|external} [pure|view|payable] [returns (<return types>)]

In contrast to the parameter types, the return types cannot be empty - if the
function type should not return anything, the whole ``returns (<return types>)``
part has to be omitted.

By default, function types are internal, so the ``internal`` keyword can be
omitted. Note that this only applies to function types. Visibility has
to be specified explicitly for functions defined in contracts, they
do not have a default.

A function type ``A`` is implicitly convertible to a function type ``B`` if and only if
their parameter types are identical, their return types are identical,
their internal/external property is identical and the state mutability of ``A``
is not more restrictive than the state mutability of ``B``. In particular:

 - ``pure`` functions can be converted to ``view`` and ``non-payable`` functions
 - ``view`` functions can be converted to ``non-payable`` functions
 - ``payable`` functions can be converted to ``non-payable`` functions

No other conversions are possible.

The rule about ``payable`` and ``non-payable`` might be a little
confusing, but in essence, if a function is ``payable``, this means that it
also accepts a payment of zero Ether, so it also is ``non-payable``.
On the other hand, a ``non-payable`` function will reject Ether sent to it,
so ``non-payable`` functions cannot be converted to ``payable`` functions.

If a function type variable is not initialized, calling it will result
in an exception. The same happens if you call a function after using ``delete``
on it.

If external function types are used outside of the context of Solidity,
they are treated as the ``function`` type, which encodes the address
followed by the function identifier together in a single ``bytes24`` type.

Note that public functions of the current contract can be used both as an
internal and as an external function. To use ``f`` as an internal function,
just use ``f``, if you want to use its external form, use ``this.f``.

Additionally, public (or external) functions also have a special member called ``selector``,
which returns the :ref:`ABI function selector <abi_function_selector>`::

    pragma solidity ^0.4.16;

    contract Selector {
      function f() public pure returns (bytes4) {
        return this.f.selector;
      }
    }

Example that shows how to use internal function types::

    pragma solidity ^0.4.16;

    library ArrayUtils {
      // internal functions can be used in internal library functions because
      // they will be part of the same code context
      function map(uint[] memory self, function (uint) pure returns (uint) f)
        internal
        pure
        returns (uint[] memory r)
      {
        r = new uint[](self.length);
        for (uint i = 0; i < self.length; i++) {
          r[i] = f(self[i]);
        }
      }
      function reduce(
        uint[] memory self,
        function (uint, uint) pure returns (uint) f
      )
        internal
        pure
        returns (uint r)
      {
        r = self[0];
        for (uint i = 1; i < self.length; i++) {
          r = f(r, self[i]);
        }
      }
      function range(uint length) internal pure returns (uint[] memory r) {
        r = new uint[](length);
        for (uint i = 0; i < r.length; i++) {
          r[i] = i;
        }
      }
    }

    contract Pyramid {
      using ArrayUtils for *;
      function pyramid(uint l) public pure returns (uint) {
        return ArrayUtils.range(l).map(square).reduce(sum);
      }
      function square(uint x) internal pure returns (uint) {
        return x * x;
      }
      function sum(uint x, uint y) internal pure returns (uint) {
        return x + y;
      }
    }

Another example that uses external function types::

    pragma solidity ^0.4.22;

    contract Oracle {
      struct Request {
        bytes data;
        function(uint) external callback;
      }
      Request[] requests;
      event NewRequest(uint);
      function query(bytes memory data, function(uint) external callback) public {
        requests.push(Request(data, callback));
        emit NewRequest(requests.length - 1);
      }
      function reply(uint requestID, uint response) public {
        // Here goes the check that the reply comes from a trusted source
        requests[requestID].callback(response);
      }
    }

    contract OracleUser {
      Oracle constant oracle = Oracle(0x1234567); // known contract
      uint exchangeRate;
      function buySomething() public {
        oracle.query("USD", this.oracleResponse);
      }
      function oracleResponse(uint response) public {
        require(
            msg.sender == address(oracle),
            "Only oracle can call this."
        );
        exchangeRate = response;
      }
    }

.. note::
    Lambda or inline functions are planned but not yet supported.

.. index:: ! type;reference, ! reference type, storage, memory, location, array, struct

Reference Types
==================

Complex types, i.e. types which do not always fit into 256 bits have to be handled
more carefully than the value-types we have already seen. Since copying
them can be quite expensive, we have to think about whether we want them to be
stored in **memory** (which is not persisting) or **storage** (where the state
variables are held).

.. _data-location:

Data location
-------------


Every complex type, i.e. *arrays* and *structs*, has an additional
annotation, the "data location", about where it is stored. There are three data locations:
``memory``, ``storage`` and ``calldata``. Calldata is only valid for parameters of external contract
functions and is required for this type of parameter. Calldata is a non-modifiable,
non-persistent area where function arguments are stored, and behaves mostly like memory.


.. note::
    Prior to version 0.5.0 the data location could be omitted, and would default to different locations
    depending on the kind of variable, function type, etc., but all complex types must now give an explicit
    data location.

Data locations are important because they change how assignments behave:
assignments between storage and memory and also to a state variable (even from other state variables)
always create an independent copy.
Assignments to local storage variables only assign a reference though, and
this reference always points to the state variable even if the latter is changed
in the meantime.
On the other hand, assignments from a memory stored reference type to another
memory-stored reference type do not create a copy.

::

    pragma solidity ^0.4.0;

    contract C {
        uint[] x; // the data location of x is storage

        // the data location of memoryArray is memory
        function f(uint[] memory memoryArray) public {
            x = memoryArray; // works, copies the whole array to storage
            uint[] storage y = x; // works, assigns a pointer, data location of y is storage
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

        function g(uint[] storage) internal pure {}
        function h(uint[] memory) public pure {}
    }

Summary
^^^^^^^

Forced data location:
 - parameters (not return) of external functions: calldata
 - state variables: storage

.. index:: ! array

.. _arrays:

Arrays
------

Arrays can have a compile-time fixed size or they can be dynamic.
For storage arrays, the element type can be arbitrary (i.e. also other
arrays, mappings or structs). For memory arrays, it cannot be a mapping and
has to be an ABI type if it is an argument of a publicly-visible function.

An array of fixed size ``k`` and element type ``T`` is written as ``T[k]``,
an array of dynamic size as ``T[]``. As an example, an array of 5 dynamic
arrays of ``uint`` is ``uint[][5]`` (note that the notation is reversed when
compared to some other languages). To access the second uint in the
third dynamic array, you use ``x[2][1]`` (indices are zero-based and
access works in the opposite way of the declaration, i.e. ``x[2]``
shaves off one level in the type from the right).

Variables of type ``bytes`` and ``string`` are special arrays. A ``bytes`` is similar to ``byte[]``,
but it is packed tightly in calldata. ``string`` is equal to ``bytes`` but does not allow
length or index access (for now).
So ``bytes`` should always be preferred over ``byte[]`` because it is cheaper.
As a rule of thumb, use ``bytes`` for arbitrary-length raw byte data and ``string``
for arbitrary-length string (UTF-8) data. If you can limit the length to a certain
number of bytes, always use one of ``bytes1`` to ``bytes32`` because they are much cheaper.

.. note::
    If you want to access the byte-representation of a string ``s``, use
    ``bytes(s).length`` / ``bytes(s)[7] = 'x';``. Keep in mind
    that you are accessing the low-level bytes of the UTF-8 representation,
    and not the individual characters!

It is possible to mark arrays ``public`` and have Solidity create a :ref:`getter <visibility-and-getters>`.
The numeric index will become a required parameter for the getter.

.. index:: ! array;allocating, new

Allocating Memory Arrays
^^^^^^^^^^^^^^^^^^^^^^^^

Creating arrays with variable length in memory can be done using the ``new`` keyword.
As opposed to storage arrays, it is **not** possible to resize memory arrays (e.g. by assigning to
the ``.length`` member). You either have to calculate the required size in advance
or create a new memory array and copy every element.

::

    pragma solidity ^0.4.16;

    contract C {
        function f(uint len) public pure {
            uint[] memory a = new uint[](7);
            bytes memory b = new bytes(len);
            assert(a.length == 7);
            assert(b.length == len);
            a[6] = 8;
        }
    }

.. index:: ! array;literals, !inline;arrays

Array Literals / Inline Arrays
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Array literals are arrays that are written as an expression and are not
assigned to a variable right away.

::

    pragma solidity ^0.4.16;

    contract C {
        function f() public pure {
            g([uint(1), 2, 3]);
        }
        function g(uint[3] memory) public pure {
            // ...
        }
    }

The type of an array literal is a memory array of fixed size whose base
type is the common type of the given elements. The type of ``[1, 2, 3]`` is
``uint8[3] memory``, because the type of each of these constants is ``uint8``.
Because of that, it was necessary to convert the first element in the example
above to ``uint``. Note that currently, fixed size memory arrays cannot
be assigned to dynamically-sized memory arrays, i.e. the following is not
possible:

::

    pragma solidity ^0.4.0;

    // This will not compile.
    contract C {
        function f() public {
            // The next line creates a type error because uint[3] memory
            // cannot be converted to uint[] memory.
            uint[] memory x = [uint(1), 3, 4];
        }
    }

It is planned to remove this restriction in the future but currently creates
some complications because of how arrays are passed in the ABI.

.. index:: ! array;length, length, push, pop, !array;push, !array;pop

Members
^^^^^^^

**length**:
    Arrays have a ``length`` member to hold their number of elements.
    Dynamic arrays can be resized in storage (not in memory) by changing the
    ``.length`` member. This does not happen automatically when attempting to access elements outside the current length. The size of memory arrays is fixed (but dynamic, i.e. it can depend on runtime parameters) once they are created.
**push**:
     Dynamic storage arrays and ``bytes`` (not ``string``) have a member function called ``push`` that can be used to append an element at the end of the array. The function returns the new length.
**pop**:
     Dynamic storage arrays and ``bytes`` (not ``string``) have a member function called ``pop`` that can be used to remove an element from the end of the array.

.. warning::
    It is not yet possible to use arrays of arrays in external functions.

.. note::
    In EVM versions before Byzantium, it was not possible to access
    dynamic arrays return from function calls. If you call functions
    that return dynamic arrays, make sure to use an EVM that is set to
    Byzantium mode.

::

    pragma solidity ^0.4.16;

    contract ArrayContract {
        uint[2**20] m_aLotOfIntegers;
        // Note that the following is not a pair of dynamic arrays but a
        // dynamic array of pairs (i.e. of fixed size arrays of length two).
        // Because of that, T[] is always a dynamic array of T, even if T
        // itself is an array.
        bool[2][] m_pairsOfFlags;

        // newPairs is stored in memory - the only possibility
        // for public function arguments
        function setAllFlagPairs(bool[2][] memory newPairs) public {
            // assignment to a storage array replaces the complete array
            m_pairsOfFlags = newPairs;
        }

        function setFlagPair(uint index, bool flagA, bool flagB) public {
            // access to a non-existing index will throw an exception
            m_pairsOfFlags[index][0] = flagA;
            m_pairsOfFlags[index][1] = flagB;
        }

        function changeFlagArraySize(uint newSize) public {
            // if the new size is smaller, removed array elements will be cleared
            m_pairsOfFlags.length = newSize;
        }

        function clear() public {
            // these clear the arrays completely
            delete m_pairsOfFlags;
            delete m_aLotOfIntegers;
            // identical effect here
            m_pairsOfFlags.length = 0;
        }

        bytes m_byteData;

        function byteArrays(bytes memory data) public {
            // byte arrays ("bytes") are different as they are stored without padding,
            // but can be treated identical to "uint8[]"
            m_byteData = data;
            m_byteData.length += 7;
            m_byteData[3] = 0x08;
            delete m_byteData[2];
        }

        function addFlag(bool[2] memory flag) public returns (uint) {
            return m_pairsOfFlags.push(flag);
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
                b[i] = byte(uint8(i));
            return b;
        }
    }


.. index:: ! struct, ! type;struct

.. _structs:

Structs
-------

Solidity provides a way to define new types in the form of structs, which is
shown in the following example:

::

    pragma solidity ^0.4.11;

    contract CrowdFunding {
        // Defines a new type with two fields.
        struct Funder {
            address addr;
            uint amount;
        }

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
            // Creates new struct and saves in storage. We leave out the mapping type.
            campaigns[campaignID] = Campaign(beneficiary, goal, 0, 0);
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
``campaigns[campaignID].amount = 0``.

.. index:: !mapping

Mappings
========

You declare mapping types with the syntax ``mapping(_KeyType => _ValueType)``.
The ``_KeyType`` can be any elementary type. This means it can be any of
the built-in value types plus ``bytes`` and ``string``. User-defined
or complex types like contract types, enums, mappings, structs and any array type
apart from ``bytes`` and ``string`` are not allowed.
``_ValueType`` can be any type, including mappings.

You can think of mappings as `hash tables <https://en.wikipedia.org/wiki/Hash_table>`_, which are virtually initialized
such that every possible key exists and is mapped to a value whose
byte-representation is all zeros, a type's :ref:`default value <default-value>`. The similarity ends there, the key data is not stored in a
mapping, only its ``keccak256`` hash is used to look up the value.

Because of this, mappings do not have a length or a concept of a key or
value being set.

Mappings are **only** allowed for state variables (or as storage reference types
in internal functions).

You can mark variables of mapping type as ``public`` and Solidity creates a
:ref:`getter <visibility-and-getters>` for you. The ``_KeyType`` becomes a
parameter for the getter. If ``_ValueType`` is a value type or a struct,
the getter returns ``_ValueType``.
If ``_ValueType`` is an array or a mapping, the getter has one parameter for
each ``_KeyType``, recursively. For example with a mapping:

::

    pragma solidity ^0.4.0;

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

``delete a`` assigns the initial value for the type to ``a``. I.e. for integers it is equivalent to ``a = 0``, but it can also be used on arrays, where it assigns a dynamic array of length zero or a static array of the same length with all elements reset. For structs, it assigns a struct with all members reset.

``delete`` has no effect on whole mappings (as the keys of mappings may be arbitrary and are generally unknown). So if you delete a struct, it will reset all members that are not mappings and also recurse into the members unless they are mappings. However, individual keys and what they map to can be deleted.

It is important to note that ``delete a`` really behaves like an assignment to ``a``, i.e. it stores a new object in ``a``.

::

    pragma solidity ^0.4.0;

    contract DeleteExample {
        uint data;
        uint[] dataArray;

        function f() public {
            uint x = data;
            delete x; // sets x to 0, does not affect data
            delete data; // sets data to 0, does not affect x which still holds a copy
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
Any integer type that can be converted to ``uint160`` can also be converted to ``address``.

Explicit Conversions
--------------------

If the compiler does not allow implicit conversion but you know what you are
doing, an explicit type conversion is sometimes possible. Note that this may
give you some unexpected behaviour so be sure to test to ensure that the
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
The result of the conversion will compare equal to the original integer.

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
    uint8 e = uint8(bytes1(a)); // d will be 0x12

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
